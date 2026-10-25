"""Observe imported-function composition through fresh native transfers."""

from __future__ import annotations

import hashlib
import json
import tempfile
from contextlib import contextmanager
from contextvars import ContextVar
from pathlib import Path

from harness.analysis.project import prepare_target
from harness.common.digests import digest
from harness.domain.ids import parse_function_id
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.io import repo_layout
from harness.naming.debt import address_of
from harness.naming.instructions import resolve_instructions
from harness.runtime.session import capture_input, run_session
from harness.toolchain.pcsx import PcsxReduxToolchain

COMPOSITION_INPUTS: ContextVar[dict | None] = ContextVar(
    "naming_composition_inputs", default=None
)
KIND = "owner-resolution-v1"


def bind_inputs(root: Path, paths: dict[str, Path]) -> dict:
    if set(paths) != {"bios", "disc", "origin", "state"}:
        raise ValueError("owner composition requires BIOS, disc, origin and state")
    return {name: capture_input(root / path) for name, path in sorted(paths.items())}


def operation(row: dict, target: str) -> dict | None:
    inputs = COMPOSITION_INPUTS.get()
    if inputs is None or row.get("kind") != "function" or not row.get("outside_payload"):
        return None
    selector = str(parse_function_id(f"{target}@{address_of(row['name']):08X}"))
    return {
        "kind": KIND,
        "target": selector,
        "command": f"semantic:{KIND}:{selector}:{digest(inputs)}",
        "inputs": inputs,
    }


@contextmanager
def retained_inputs(semantic: list):
    records = [item for item in semantic if "composition_id" in item]
    if len(records) > 1:
        raise ValueError("owner composition operation is duplicated")
    inputs = records[0].get("composition_inputs") if records else None
    if inputs is not None:
        if not isinstance(inputs, dict) or set(inputs) != {"bios", "disc", "origin", "state"}:
            raise ValueError("owner composition inputs are incomplete")
        current = bind_inputs(Path("/"), {key: Path(value["path"]) for key, value in inputs.items()})
        if digest(current) != digest(inputs):
            raise ValueError("owner composition inputs changed")
    token = COMPOSITION_INPUTS.set(inputs)
    try:
        yield
    finally:
        COMPOSITION_INPUTS.reset(token)


def _capture(directory: Path, receipt: dict, name: str) -> bytes:
    entries = [item for item in receipt["completion"]["captures"] if item["path"] == name]
    if len(entries) != 1:
        raise ValueError(f"native composition capture is missing or duplicated: {name}")
    path = directory / name
    if path.is_symlink() or path.resolve() != path:
        raise ValueError("native composition capture must be a canonical regular file")
    raw = path.read_bytes()
    if len(raw) != entries[0]["bytes"] or hashlib.sha256(raw).hexdigest() != entries[0]["sha256"]:
        raise ValueError("native composition capture changed")
    return raw


def _ram_slice(ram: bytes, address: int, size: int) -> bytes:
    if len(ram) != 2097152 or not 0x80000000 <= address < 0x80200000:
        raise ValueError("composition requires canonical cached first-2MiB RAM")
    offset = address - 0x80000000
    if offset + size > len(ram):
        raise ValueError("composition range escapes physical RAM")
    return ram[offset : offset + size]


def _body(root: Path, selector: str, before: bytes, after: bytes) -> dict:
    binding, original = resolve_instructions(root, selector)
    for ram in (before, after):
        if _ram_slice(ram, binding["start"], len(original)) != original:
            raise ValueError(f"complete reviewed body differs from RAM: {selector}")
    return {"binding": binding, "sha256": hashlib.sha256(original).hexdigest()}


def _candidate_images(root: Path, address: int, ram: bytes) -> list[dict]:
    candidates = []
    for target, manifest in sorted(load_target_manifests(root).items()):
        if manifest.load_address > address:
            continue
        spec = prepare_target(root, target, manifest=manifest)
        image = spec.binary.read_bytes()
        end = spec.load_address + len(image) - spec.binary_offset
        if not spec.load_address <= address < end:
            continue
        layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
        boundary = layout.find_boundary_at(address)
        if boundary is None or boundary.virtual_end is None:
            raise ValueError(f"unresolved plausible owner interval: {target}")
        start, stop = boundary.virtual_start, boundary.virtual_end
        if start < spec.load_address or stop > end or not 0 < stop - start <= 65536:
            raise ValueError(f"unavailable complete plausible owner interval: {target}")
        offset = spec.binary_offset + start - spec.load_address
        original = image[offset : offset + stop - start]
        observed = _ram_slice(ram, start, len(original))
        candidates.append({
            "target": target, "start": start, "end": stop,
            "function": boundary.is_function,
            "binary_sha256": hashlib.sha256(image).hexdigest(),
            "splat_sha256": layout.sha256,
            "original_sha256": hashlib.sha256(original).hexdigest(),
            "observed_sha256": hashlib.sha256(observed).hexdigest(),
            "matches": observed == original,
        })
    return candidates


def _check_steps(rows: list, pc: int, destination: int, registers: dict) -> dict:
    if len(rows) != 3 or set(rows[0]) != {"initial"}:
        raise ValueError("native composition requires exactly two observed transitions")
    initial, first, second = rows[0]["initial"], rows[1], rows[2]
    if initial != first["before"] or first["after"] != second["before"]:
        raise ValueError("native composition step contexts are discontinuous")
    contexts = (initial, first["after"], second["after"])
    if [item["pc"] for item in contexts] != [pc, pc + 4, destination]:
        raise ValueError("native composition transfer PCs disagree")
    if initial["gpr"][:32] != registers["registers"] or int(initial["cycles"]) != registers["cycles"]:
        raise ValueError("native composition restoration changed registers or cycles")
    jal = 0x0C000000 | ((destination >> 2) & 0x03FFFFFF)
    if initial["next_memory_word"] != jal or first["after"]["last_executed_word"] != jal:
        raise ValueError("native composition did not execute the original JAL")
    if first["after"]["next_memory_word"] != 0 or second["after"]["last_executed_word"] != 0:
        raise ValueError("native composition supports only an original NOP slot")
    if [item["next_is_delay_slot"] for item in contexts] != [False, True, False]:
        raise ValueError("native composition delay-slot state disagrees")
    for number, transition in enumerate((first, second), 1):
        before, after = transition["before"], transition["after"]
        if (transition["step"] != number or transition["native_exception_pause"]
                or transition["exception_context_changed"] or before["cp0"] != after["cp0"]
                or before["exception"] != after["exception"]
                or int(after["cycles"]) - int(before["cycles"]) != 2):
            raise ValueError("native composition crossed an unsupported transition")
        expected = list(before["gpr"])
        if number == 1:
            expected[31] = pc + 8
        if after["gpr"] != expected:
            raise ValueError("native composition changed unexpected registers")
    for index, context in enumerate(contexts):
        slots = [context["delay_slot_info_1"], context["delay_slot_info_2"]]
        pending = [slot for slot in slots if slot["pc_active"]]
        if context["in_isr"] or any(slot["active"] for slot in slots):
            raise ValueError("native composition has ISR or pending-load state")
        if index == 1:
            if len(pending) != 1 or pending[0]["pc_value"] != destination or not pending[0]["from_link"]:
                raise ValueError("native composition pending JAL target disagrees")
        elif pending:
            raise ValueError("native composition has an unexpected pending branch")
    return {"initial": initial, "transitions": [first, second]}


def capture_composition(root: Path, op: dict, row: dict, parent: Path) -> dict:
    """Execute the code-owned native sequence; earlier receipts supply no facts."""
    if operation(row, op["target"].split("@", 1)[0]) != op:
        raise ValueError("owner composition operation differs from generated work")
    paths = {key: Path(value["path"]) for key, value in op["inputs"].items()}
    if digest(bind_inputs(root, paths)) != digest(op["inputs"]):
        raise ValueError("owner composition input binding changed")
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix="owner-resolution-", dir=parent)).resolve()
    toolchain = PcsxReduxToolchain(repo_layout(root))
    executions = []

    def run(name, mission, state, origin, arguments):
        directory = output / name
        receipt = run_session(toolchain, bios=paths["bios"], disc=paths["disc"],
            mission=mission, output=directory, origin=origin,
            files=[f"state={state}"], arguments=arguments)
        if receipt["status"] != "passed" or receipt["process"]["exit_code"] != 0:
            raise ValueError(f"native composition {name} did not succeed; retain {directory}")
        executions.append({"directory": str(directory), "receipt": capture_input(directory / "receipt.json")})
        return directory, receipt

    start_dir, start_receipt = run("start", "target", paths["state"], paths["origin"], ["savestate=1"])
    before = _capture(start_dir, start_receipt, "ram.bin")
    registers = json.loads(_capture(start_dir, start_receipt, "registers.json"))
    pc, destination = registers["pc"], address_of(row["name"])
    callers = []
    for work in row.get("required_work", []):
        if work.get("status") != "open" or not str(work.get("id", "")).startswith("caller:"):
            continue
        selector = str(parse_function_id(work["id"].split(":", 1)[1]))
        binding, original = resolve_instructions(root, selector)
        if binding["start"] <= pc and pc + 8 <= binding["end"]:
            callers.append(selector)
    if len(callers) != 1:
        raise ValueError("supplied state PC is not within one generated reviewed caller")
    caller = callers[0]
    jal = 0x0C000000 | ((destination >> 2) & 0x03FFFFFF)
    if pc % 4 or (pc + 4) & 0xF0000000 != destination & 0xF0000000 or _ram_slice(before, pc, 8) != jal.to_bytes(4, "little") + b"\0" * 4:
        raise ValueError("supplied state must already pause at the selected original JAL and NOP")
    _body(root, caller, before, before)
    candidates = _candidate_images(root, destination, before)
    matches = [candidate for candidate in candidates if candidate["matches"]]
    owners = {str(parse_function_id(work["id"].split(":", 1)[1])) for work in row.get("required_work", [])
        if work.get("status") == "open" and str(work.get("id", "")).startswith("owner:")}
    if len(matches) != 1 or not matches[0]["function"] or matches[0]["start"] != destination:
        raise ValueError("complete plausible owner intervals do not distinguish one reviewed function entry")
    owner = str(parse_function_id(f"{matches[0]['target']}@{destination:08X}"))
    if owner not in owners:
        raise ValueError("observed owner is not generated owner work")
    _body(root, owner, before, before)
    step_dir, step_receipt = run("transfer", "step", start_dir / "state.pbuf", start_dir / "receipt.json",
        [f"identity={KIND}", "steps=2", f"stop=0x{destination:08X}", "exceptions=stop", "savestate=1"])
    steps = [json.loads(line) for line in _capture(step_dir, step_receipt, "steps.ndjson").decode().splitlines()]
    step_report = json.loads(_capture(step_dir, step_receipt, "step.json"))
    if step_report.get("success") is not True or step_report.get("steps") != 2 or step_report.get("reason") != "stop reached":
        raise ValueError("native composition did not reach the owner with two supported steps")
    transfer = _check_steps(steps, pc, destination, registers)
    end_dir, end_receipt = run("entry", "target", step_dir / "state.pbuf", step_dir / "receipt.json", ["savestate=1"])
    after = _capture(end_dir, end_receipt, "ram.bin")
    final = json.loads(_capture(end_dir, end_receipt, "registers.json"))
    context = steps[-1]["after"]
    if final["pc"] != destination or final["cycles"] != int(context["cycles"]) or final["registers"] != context["gpr"][:32]:
        raise ValueError("native composition entry restoration changed the stopped context")
    if _candidate_images(root, destination, after) != candidates:
        raise ValueError("native composition owner candidates changed during transfer")
    value = {"inputs": op["inputs"], "caller_contract": caller,
        "caller": _body(root, caller, before, after), "owner_contract": owner,
        "owner": _body(root, owner, before, after), "candidates": candidates,
        "transfer": transfer,
        "authority": "fresh builtin native transfer from the identified supplied state",
        "scope": "original JAL and NOP transfer to an entry whose complete RAM range matches the reviewed owner contract",
        "assumptions": ["standard stable RAM mappings and ordinary native execution"],
        "limits": ["caller matching is not exclusive caller-image identity", "owner instructions and instruction-cache equality are unobserved", "no complete-image residency or authenticated history", "candidate exclusion proves no code/data interpretation"]}
    raw = json.dumps({"value": value, "executions": executions}, sort_keys=True)
    return {"composition_id": op["command"], "composition_inputs": op["inputs"],
        "composition_value": value, "command": op["command"], "selector": op["target"],
        "exit": 0, "killed": False, "semantic_success": True,
        "output": raw, "raw": raw, "stderr": ""}


def observations(semantic: list) -> tuple[dict, ...]:
    return tuple({"class": "owner_resolution", "polarity": "positive", "value": item["composition_value"]}
        for item in semantic if "composition_id" in item)


def projection(values: tuple[dict, ...]) -> dict:
    return {"schema": "bof3.naming-evidence-facts/v1", "facts": ["owner_resolution"], "observations": list(values), "open": []}


def replay_composition(root: Path, row: dict, target: str, payload: dict) -> tuple[dict, ...]:
    records = [item for item in payload["semantic"] if "composition_id" in item]
    if not records:
        return ()
    with retained_inputs(payload["semantic"]):
        op = operation(row, target)
        saved = records[0]
        if op is None or any(saved.get(key) != value for key, value in {
                "composition_id": op["command"], "command": op["command"], "selector": op["target"],
                "exit": 0, "killed": False, "semantic_success": True}.items()):
            raise ValueError("retained owner composition differs from its generated operation")
        raw_path = root / saved["raw_file"]
        raw = raw_path.read_text()
        value = json.loads(raw)
        if digest(value["value"]) != digest(saved["composition_value"]):
            raise ValueError("retained owner composition output differs from its projection")
        commands = [item for item in payload["commands"] if item.get("command") == op["command"]]
        if len(commands) != 1 or commands[0].get("output") != raw or commands[0].get("status") != "passed" or commands[0].get("selector") != op["target"]:
            raise ValueError("retained owner composition command record differs")
        receipt = json.loads((root / commands[0]["receipt"]).read_text())
        expected = projection(observations([saved]))
        if any(digest(receipt.get(key)) != digest(item) for key, item in expected.items()):
            raise ValueError("retained owner composition typed receipt differs")
        fresh = capture_composition(root, op, row, raw_path.parent / "runtime")
        if digest(fresh["composition_value"]) != digest(saved["composition_value"]):
            raise ValueError("fresh native owner composition differs from retained facts")
        return observations([fresh])
