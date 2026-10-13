"""Capture and authenticate original-only and freshly supplied source evidence."""

from __future__ import annotations

import ast
from collections.abc import Mapping
from dataclasses import asdict, dataclass, replace
import hashlib
import json
import math
from pathlib import Path
import re
import struct
import time
from types import MappingProxyType

from harness.build import recipe
from harness.common.artifacts import (
    FileIdentity,
    active_reader,
    encode_json,
    physical_read_bytes,
    read_bytes,
    using_reader,
    write_bytes,
)
from harness.common.deadlines import check_deadline, resolve_deadline, use_deadline
from harness.common.digests import file_record
from harness.common.captureproof import CaptureProofRefused, verify_capture
from harness.common.failures import FailureEvent, PolicyFailure, facts, primary, retain
from harness.common.process import ProcessCleanupError, capture
from harness.match import extraction
from harness.reconstruction import (
    bundles,
    catalog,
    calltargets,
    contract,
    nativeoutput,
    prelude,
    provenance,
)
from harness.reconstruction.obligations import PredicateResult
from harness.reconstruction.prepare import PreparedInputs


@dataclass(frozen=True)
class NativeCommand:
    cwd: Path
    argv: tuple[str, ...]
    environment: Mapping[str, str]
    deadline: float
    output_limit: int
    timeout: float


@dataclass(frozen=True)
class OriginalInput:
    member: provenance.BoundMember
    image: provenance.BoundImage
    original: bytes
    payload: bytes
    scope: dict
    contexts: calltargets.NativeContexts
    overlay_mounts: tuple[str, ...] = ()
    entry_observation: EntryObservation | None = None


@dataclass(frozen=True)
class EntryObservation:
    descriptor: dict
    files: Mapping[str, bytes]


def capture_entry_observation(entry_receipt, cpu_receipt, contexts, mounts):
    from harness.runtime import completion, origin

    receipts, files, sources = [], {}, []
    for name, path, mission in (
        ("entry", Path(entry_receipt), "target"),
        ("cpu", Path(cpu_receipt), "state"),
    ):
        raw = read_bytes(path, 2 << 20)
        receipt = json.loads(raw, object_pairs_hook=origin.decode_object)
        _require(
            receipt.get("status") == "passed"
            and origin.validate_selection(receipt).get("name") == mission
            and completion.validate_completion(path.parent) == receipt["completion"],
            "entry observation requires passed native target and CPU state captures",
        )
        receipts.append(receipt)
        files[f"{name}-receipt.json"] = raw
        sources.append(
            {
                "path": str(path.resolve()),
                "bytes": len(raw),
                "sha256": hashlib.sha256(raw).hexdigest(),
            }
        )
    entry, cpu = receipts
    _require(
        cpu.get("arguments") == {"DEPTH": "2", "PATH": "registers"}
        and "file:STATE" in cpu["inputs"]
        and set(cpu["inputs"]) <= {"bios", "file:STATE"}
        and all(key == "bios" or key.startswith("disc:") for key in entry["inputs"]),
        "CPU observation must inspect the entry state without execution or injection",
    )
    continuation = origin.verify_origin(
        Path(entry_receipt), {**entry["inputs"], **cpu["inputs"]}, cpu["emulator"]
    )
    state = cpu["inputs"]["file:STATE"]
    recipe.verify_inputs([state])
    sources.append(state)
    for name, receipt_path, receipt, capture_name, limit in (
        ("registers.json", entry_receipt, entry, "registers.json", 64 << 10),
        ("ram.bin", entry_receipt, entry, "ram.bin", 2 << 20),
        ("cpu.json", cpu_receipt, cpu, "query.json", 64 << 10),
    ):
        record = next(
            row
            for row in receipt["completion"]["captures"]
            if row["path"] == capture_name
        )
        path = Path(receipt_path).parent / capture_name
        raw = read_bytes(path, limit)
        _require(
            len(raw) == record["bytes"]
            and hashlib.sha256(raw).hexdigest() == record["sha256"],
            "entry capture changed while consumed",
        )
        files[name] = raw
        sources.append({**record, "path": str(path.resolve())})
    registers = nativeoutput.parse_json(files["registers.json"], 64 << 10)
    pc = registers["pc"]
    _require(
        registers.get("schema") == "psx.runtime-registers/v1"
        and type(pc) is int
        and pc % 4 == 0
        and entry["arguments"] == {"TARGET": f"0x{pc:08x}", "SAVESTATE": "1"}
        and len(files["ram.bin"]) == 2 << 20,
        "entry capture PC or RAM differs",
    )
    candidates = []
    for image_id in mounts:
        image = next(image for image in contexts.images if image.image_id == image_id)
        if image.load_address <= pc < image.load_address + image.payload_size:
            candidates.append(image)
    _require(candidates, "observed entry has no mounted original image owner")
    image = candidates[-1]
    targets = [
        target.target
        for target in contexts.targets
        if target.image_id == image.image_id
    ]
    _require(len(targets) == 1, "observed image target is ambiguous")
    descriptor = {
        "function": f"{targets[0]}@{pc:08x}",
        "image_id": image.image_id,
        "image_sha256": image.file_sha256,
        "entry_pc": pc,
        "state": {key: state[key] for key in ("bytes", "sha256")},
        "files": {
            name: {
                "path": f"input/entry-{name}",
                "bytes": len(raw),
                "sha256": hashlib.sha256(raw).hexdigest(),
            }
            for name, raw in files.items()
        },
    }
    _require(
        continuation["state_sha256"] == descriptor["state"]["sha256"],
        "entry state differs",
    )
    recipe.verify_inputs(sources)
    return EntryObservation(descriptor, MappingProxyType(files)), tuple(sources)


def read_entry_observation(descriptor, directory, *, reader=None):
    if descriptor is None:
        return None
    nativeoutput.validate_entry_observation_descriptor(descriptor)
    files = {}
    for name, record in descriptor["files"].items():
        _require(record["path"] == f"input/entry-{name}", "entry input path differs")
        path = Path(directory) / f"entry-{name}"
        raw = (reader or physical_read_bytes)(path, record["bytes"], [])
        _require(
            len(raw) == record["bytes"]
            and hashlib.sha256(raw).hexdigest() == record["sha256"],
            "retained entry input differs",
        )
        files[name] = raw
    return EntryObservation(descriptor, MappingProxyType(files))


@dataclass(frozen=True)
class NativeAttempt:
    unit: nativeoutput.CapturedUnit | None
    evidence: dict
    pending: Exception | None
    events: tuple[FailureEvent, ...] = ()

    @property
    def required_files(self):
        return (
            tuple(self.evidence.get("expected_inputs", ()))
            + tuple(self.evidence.get("expected_outputs", ()))
            + tuple(
                {
                    **record,
                    "path": str(
                        Path(self.evidence["workspace"]) / name / record["path"]
                    ),
                }
                for name, records in {
                    **self.evidence.get("inputs", {}),
                    "work": self.evidence.get("outputs", ()),
                }.items()
                for record in records
            )
        )

    @property
    def launch(self):
        return self.evidence["launch"]

    @property
    def status(self):
        return self.evidence.get("status", {}).get("status")

    @property
    def raw_exit(self):
        return self.evidence.get("process", {}).get("raw_exit")

    @property
    def process_failure(self):
        return self.evidence.get("process", {}).get("failure")


@dataclass(frozen=True)
class NativeInspection:
    unit: nativeoutput.CapturedUnit | None
    authenticated_mode: str | None
    predicates: tuple
    events: tuple[FailureEvent, ...]


def inspect_rebuild_native(
    rebuilt, run, binding, prepared_run, limit, events, *, reader=None
) -> nativeoutput.CapturedUnit | None:
    if run.get("producer", {}).get("kind") == SUPPLIED_KIND:
        return None
    from harness.reconstruction.nativegeneration import _adapt_retained_native

    reader = reader or active_reader() or physical_read_bytes
    record = rebuilt["submitted_result"]
    _require(record["bytes"] <= limit, "rebuild native result exceeds its read bound")
    _, raw = _consume_identity(record, reader, events)
    result = _adapt_retained_native(
        nativeoutput.parse_json(raw, limit), run, record, reader=reader
    )
    native = inspect_native(
        Path(record["path"]).parent, result, run, binding, prepared_run, reader
    )
    events.extend(native.events)
    unit = native.unit
    _require(
        unit is not None
        and not native.events
        and asdict(unit.member) == rebuilt["identity"]
        and unit.entry_symbol == rebuilt["entry_symbol"]
        and len(unit.source) == rebuilt["source"]["bytes"]
        and hashlib.sha256(unit.source).hexdigest() == rebuilt["source"]["sha256"],
        "rebuild native source does not reproduce its member, entry and source",
    )
    return unit


def inspect_native(candidate, result, run, binding, prepared_run, reader):
    from harness.reconstruction import nativegeneration

    events, unit, mode = [], None, None
    try:
        member = binding.member(result["identity"]["key"])
        _require(result["identity"] == asdict(member), "native member identity differs")
        image, original, payload = nativegeneration._original(
            binding, prepared_run, member, events
        )
        contexts_record = run["binding_inputs"]
        catalog_record = next(
            row
            for row in contexts_record["files"]
            if row["path"] == "image-catalog.json"
        )
        contexts = calltargets.NativeContexts(
            binding.problem_doc_sha256,
            catalog_record["sha256"],
            tuple(sorted(binding.images, key=lambda item: item.image_id)),
            tuple(
                sorted(binding.targets, key=lambda item: (item.target, item.image_id))
            ),
            tuple(sorted(binding.members, key=lambda item: item.key)),
            Path(contexts_record["directory"]),
        )
        scope = nativegeneration._call_scope(binding, prepared_run, member, events)
        original_input = OriginalInput(
            member,
            image,
            original,
            payload,
            scope,
            contexts,
            tuple(run.get("overlay_mounts", ())),
            read_entry_observation(
                run.get("entry_observation"), candidate / "input", reader=reader
            ),
        )
        native = result["native"]
        if native.get("process_origin", "direct") == "direct":
            process = native["process"]
            if type(process["argv"]) is list:
                native = {
                    **native,
                    "process": {**process, "argv": tuple(process["argv"])},
                }
        source_form = (
            nativeoutput.parse_source_form(run["source_form"])
            if "source_form" in run
            else None
        )
        command = nativegeneration._native_command(
            candidate,
            Path(run["binaries"]["rizin"]["path"]),
            Path(run["binaries"]["plugin"]["path"]),
            run["limits"]["deadline"],
            run["limits"]["process_output_bytes"],
        )
        events.extend(
            verify_native(
                command,
                original_input,
                NativeAttempt(None, native, None),
                source_form=source_form,
            )
        )
        if primary(events) is not None:
            raise primary(events)
        _require(
            native.get("launch") == "observed"
            and native.get("outcome") == "generated"
            and native.get("process", {}).get("raw_exit") == 0
            and native["process"].get("failure") is None,
            "native generation does not reproduce a successful owned execution",
        )
        request = reader(candidate / "input/request.json", 64 << 10, events)
        config = reader(
            candidate / "native/fixed-config.json", nativeoutput.CONFIG_LIMIT, events
        )
        status = reader(
            candidate / "work/status.json", nativeoutput.STATUS_LIMIT, events
        )
        tokens = reader(
            candidate / "work/tokens.json", nativeoutput.TOKEN_LIMIT, events
        )
        inferred = reader(
            candidate / "work/inferred-config.json", nativeoutput.CONFIG_LIMIT, events
        )
        bindings_raw = reader(
            candidate / "work/bindings.json", calltargets.BINDINGS_LIMIT, events
        )
        observed = nativeoutput.validate_status(status, request, config)
        bindings = nativeoutput.validate_bindings(
            bindings_raw, observed, request, contexts, payload
        )
        unit = nativeoutput.decode_unit(
            member=member,
            image=image,
            request=request,
            configuration=config,
            status=status,
            tokens=tokens,
            inferred=inferred,
            payload=payload,
            expected_scope=scope,
            bindings=bindings,
            contexts=replace(contexts, input_directory=candidate / "input"),
            entry_captures=original_input.entry_observation.files
            if original_input.entry_observation
            else None,
        )
        _require(
            reader(candidate / "generated.c", nativeoutput.SOURCE_LIMIT, events)
            == unit.source,
            "actual generated source differs from complete native tokens",
        )
        nativeoutput._clean_input(observed)
        mode = "autonomous"
    except Exception as error:
        retain(events, error, phase="native-inspection", path=candidate)
    return NativeInspection(
        unit,
        mode,
        (
            PredicateResult(
                "generation",
                "matched" if unit is not None and not events else "failed",
                "reproduced native execution, full original/config/token/source identities",
            ),
            PredicateResult(
                "mode",
                "matched" if mode else "unknown",
                "actual fixed configuration, zero imported aids, original catalog and staged dependency closure"
                if mode
                else "authenticated delivered inputs do not establish a recognized mode",
            ),
        ),
        tuple(events),
    )


def _require(condition, message):
    if not condition:
        raise PolicyFailure(message)


def _read(path, limit, events=None):
    data = read_bytes(path, limit, events)
    _require(len(data) <= limit, f"file exceeds read bound: {path}")
    return data


def _load(path, limit, events):
    _require(not path.is_symlink() and path.is_file(), "retained JSON file differs")
    raw = _read(path, limit, events)
    value = nativeoutput.parse_json(raw, limit)
    _require(
        raw == encode_json(value, events=events, path=path),
        "retained JSON bytes differ",
    )
    return value


def verify_native(command, original, attempt, *, receipt=True, source_form=None):
    events = []
    result, workspace = attempt.evidence, command.cwd
    try:
        request_path = workspace / "input/request.json"
        if (
            "inputs" in result
            or "process" in result
            or request_path.exists()
            or request_path.is_symlink()
        ):
            _require(
                _read(request_path, 64 << 10, events)
                == _request(
                    command.deadline, original, events, source_form=source_form
                ),
                "native retained request differs from original inputs, cutoff or source form",
            )
    except Exception as error:
        retain(
            events,
            error,
            phase="request-binding",
            path=workspace / "input/request.json",
        )
    for key in ("expected_inputs", "expected_outputs"):
        try:
            records = result.get(key, [])
            _require(type(records) is list, "native expected inventory differs")
        except Exception as error:
            retain(
                events,
                error,
                phase="inventory",
                path=workspace,
                origin="policy"
                if isinstance(error, (PolicyFailure, recipe.ImmutableInputDrift))
                else "unknown",
            )
            continue
        for record in records:
            try:
                _require(
                    type(record) is dict and type(record.get("path")) is str,
                    "native file record differs",
                )
                path = Path(record["path"])
                _require(
                    path.is_relative_to(workspace) and ".." not in path.parts,
                    "native path escapes workspace",
                )
                required = (
                    "inputs" in result
                    if key == "expected_inputs"
                    else "source" in result
                )
                if required or path.exists() or path.is_symlink():
                    _require(
                        not path.is_symlink() and path.is_file(),
                        "native retained file differs",
                    )
                    mode = 0o600 if path == workspace / "pcs.json" else 0o400
                    _require(
                        path.stat().st_mode & 0o777 == mode,
                        "native staged copy mode differs",
                    )
                    recipe.verify_inputs([record], events=events)
            except Exception as error:
                retain(
                    events,
                    error,
                    phase="identity",
                    path=workspace,
                    origin="policy"
                    if isinstance(error, (PolicyFailure, recipe.ImmutableInputDrift))
                    else "unknown",
                )
    if original is not None:
        for directory in (original.contexts.input_directory, workspace / "input"):
            try:
                if directory == original.contexts.input_directory or "inputs" in result:
                    calltargets.verify_native_contexts(
                        original.contexts, directory=directory
                    )
            except Exception as error:
                retain(events, error, phase="contexts", path=directory)
    for group in ("inputs", "outputs"):
        try:
            groups = (
                result.get("inputs", {})
                if group == "inputs"
                else {"work": result["outputs"]}
                if "outputs" in result
                else {}
            )
            _require(type(groups) is dict, "native tree inventory differs")
        except Exception as error:
            retain(events, error, phase="inventory", path=workspace)
            continue
        for name, records in groups.items():
            try:
                _require(
                    name in {"input", "native", "work"} and type(records) is list,
                    "native tree path/inventory differs",
                )
                _require(
                    records == PreparedInputs.tree(workspace / name),
                    "native retained tree differs",
                )
            except Exception as error:
                retain(
                    events,
                    error,
                    phase="tree",
                    path=workspace,
                    origin="policy" if isinstance(error, PolicyFailure) else "unknown",
                )
            if type(records) is not list or name not in {"input", "native", "work"}:
                continue
            for record in records:
                try:
                    _require(
                        type(record) is dict
                        and type(record.get("path")) is str
                        and not Path(record["path"]).is_absolute()
                        and ".." not in Path(record["path"]).parts,
                        "native tree record path differs",
                    )
                    recipe.verify_inputs(
                        [{**record, "path": str(workspace / name / record["path"])}],
                        events=events,
                    )
                except Exception as error:
                    retain(
                        events,
                        error,
                        phase="tree-identity",
                        path=workspace / name,
                        origin="policy"
                        if isinstance(
                            error, (PolicyFailure, recipe.ImmutableInputDrift)
                        )
                        else "unknown",
                    )
    for name, limit in (
        ("native-result.json", 1 << 20),
        ("bindings-observation.json", 16 << 10),
    ):
        if name == "native-result.json" and not receipt:
            continue
        try:
            path = workspace / name
            expected = (
                result if name == "native-result.json" else result.get("bindings")
            )
            if path.exists() or path.is_symlink():
                recovered = _load(path, limit, events)
                _require(
                    expected is not None
                    and encode_json(recovered, events=events, path=path)
                    == encode_json(expected, events=events, path=path),
                    "native receipt differs from retained evidence",
                )
                if name == "bindings-observation.json":
                    recipe.verify_inputs([recovered["artifact"]], events=events)
            else:
                _require(expected is None, "native observation receipt is missing")
        except Exception as error:
            retain(
                events,
                error,
                phase="recovery",
                path=workspace / name,
                origin="policy"
                if isinstance(error, (PolicyFailure, recipe.ImmutableInputDrift))
                else "unknown",
            )
    if "process" in result:
        try:
            launch = verify_capture(
                result["process"],
                workspace,
                command.argv,
                environment=command.environment,
                deadline=command.deadline,
                output_limit=command.output_limit,
                name="native",
                timeout=command.timeout,
                origin=result.get("process_origin", "direct"),
                events=events,
                reader=active_reader(),
            )
            _require(launch == result["launch"], "native capture launch differs")
        except Exception as error:
            retain(
                events,
                error,
                phase="proof",
                path=workspace / "native.json",
                origin="policy"
                if isinstance(
                    error,
                    (PolicyFailure, recipe.ImmutableInputDrift, CaptureProofRefused),
                )
                else "unknown",
            )
    return tuple(events)


def _request(cutoff, original, events, *, source_form=None):
    _require(
        source_form is None or type(source_form) is nativeoutput.SourceForm,
        "native source form must be validated",
    )
    _require(
        source_form is None or original is not None,
        "self-check cannot select a source form",
    )
    body = {
        "schema": "retdec118.native-request/v1",
        "operation": "self-check" if original is None else "generate",
        "config_sha256": nativeoutput.CONFIG_SHA256,
        "cutoff": cutoff,
        "member": None,
        "image": None,
    }
    if original is not None:
        image, payload = original.image, original.payload
        body["member"] = {
            name: getattr(original.member, name) for name in calltargets.MEMBER_FIELDS
        }
        body["image"] = {
            "file_sha256": image.file_sha256,
            "file_bytes": image.bytes,
            "payload_sha256": hashlib.sha256(payload).hexdigest(),
            "payload_offset": image.payload_offset,
            "payload_bytes": len(payload),
            "load_address": image.load_address,
        }
        body.update(schema="retdec118.native-request/v2", **original.scope)
        body.update(
            schema="retdec118.native-request/v3",
            image_catalog_sha256=original.contexts.catalog_sha256,
        )
        if source_form is not None:
            body.update(
                schema="retdec118.native-request/v4", source_form=source_form.value
            )
        if original.overlay_mounts:
            body.update(
                schema="retdec118.native-request/v5",
                overlay_mounts=list(original.overlay_mounts),
            )
        if original.entry_observation is not None:
            body.update(
                schema="retdec118.native-request/v6",
                entry_observation=original.entry_observation.descriptor,
            )
    body["request_id"] = hashlib.sha256(
        encode_json(body, events=events, path="native")
    ).hexdigest()
    return encode_json(body, events=events, path="native")


def capture_native(
    command: NativeCommand,
    *,
    configuration: bytes,
    original: OriginalInput | None,
    source_form: nativeoutput.SourceForm | None = None,
) -> NativeAttempt:
    workspace, work = command.cwd, command.cwd / "work"
    result, events, expected = (
        {"launch": "prelaunch", "workspace": str(workspace)},
        [],
        [],
    )
    modes = {}
    phase, path = "prepare", workspace
    unit = None
    try:
        request = _request(command.deadline, original, events, source_form=source_form)
        files = {
            "input/request.json": request,
            "native/fixed-config.json": configuration,
        }
        if original is not None:
            files.update(
                {
                    "input/image.raw": original.payload,
                    "input/original.image": original.original,
                }
            )
            if original.entry_observation is not None:
                files.update(
                    {
                        f"input/entry-{name}": raw
                        for name, raw in original.entry_observation.files.items()
                    }
                )
        expected = [
            {
                "path": str(workspace / name),
                "bytes": len(data),
                "sha256": hashlib.sha256(data).hexdigest(),
            }
            for name, data in files.items()
        ]
        if original is not None:
            contexts = original.contexts
            catalog = _read(
                contexts.input_directory / "image-catalog.json",
                calltargets.CATALOG_LIMIT,
                events,
            )
            expected.append(
                dict(
                    path=str(workspace / "input/image-catalog.json"),
                    bytes=len(catalog),
                    sha256=contexts.catalog_sha256,
                )
            )
            expected.extend(
                dict(
                    path=str(workspace / "input/images" / f"{index:03d}.image"),
                    bytes=image.bytes,
                    sha256=image.file_sha256,
                )
                for index, image in enumerate(contexts.images)
            )
        result["expected_inputs"] = list(expected)
        modes.update({record["path"]: 0o400 for record in expected})
        for name in ("input", "native", "work", "home", "config"):
            try:
                (workspace / name).mkdir(mode=0o700, parents=True)
            except Exception as error:
                retain(
                    events,
                    error,
                    phase="mkdir",
                    path=workspace / name,
                    origin="ordinary",
                )
                raise
        for name, data in files.items():
            phase, path = "stage", workspace / name
            write_bytes(path, data, readonly=True, events=events)
        if original is not None:
            phase, path = "stage-contexts", workspace / "input"
            calltargets.stage_native_contexts(original.contexts, workspace / "input")
        recipe.verify_inputs(expected, events=events)
        result["inputs"] = {
            name: PreparedInputs.tree(workspace / name) for name in ("input", "native")
        }
        phase, path = "receipt", workspace / "staged-inputs.json"
        write_bytes(
            path, encode_json(result["inputs"], events=events, path=path), events=events
        )
        phase, path = "capture", workspace / "native.json"
        result["launch"] = "unknown"
        process = capture(
            workspace,
            command.argv,
            environment=command.environment,
            deadline=command.deadline,
            output_limit=command.output_limit,
            name="native",
            timeout=command.timeout,
        )
        result["process"] = process
        phase, path = "proof", workspace / "native.json"
        result["launch"] = verify_capture(
            process,
            workspace,
            command.argv,
            environment=command.environment,
            deadline=command.deadline,
            output_limit=command.output_limit,
            name="native",
            timeout=command.timeout,
            origin="direct",
            events=events,
            reader=active_reader(),
        )
        _require(
            process["owner_observation"] is None,
            "native capture returned unresolved ownership",
        )
        phase, path = "artifacts", work
        for path in work.rglob("*"):
            if path.is_file():
                _read(path, 64 << 20, events)
        result["outputs"] = PreparedInputs.tree(work)
        expected.extend(
            {**record, "path": str(work / record["path"])}
            for record in result["outputs"]
        )
        phase, path = "status", work / "status.json"
        status_bytes = _read(work / "status.json", nativeoutput.STATUS_LIMIT, events)
        status = nativeoutput.validate_status(status_bytes, request, configuration)
        result["status"] = status
        phase, path = "receipt", workspace / "status-observation.json"
        write_bytes(path, encode_json(status, events=events, path=path), events=events)
        artifacts = {}
        for prefix, name, limit in (
            ("tokens", "tokens.json", nativeoutput.TOKEN_LIMIT),
            (
                "inferred_configuration",
                "inferred-config.json",
                nativeoutput.CONFIG_LIMIT,
            ),
            ("bindings", "bindings.json", calltargets.BINDINGS_LIMIT),
        ):
            if status.get(prefix + "_complete", False):
                phase, path = "artifact", work / name
                raw = _read(path, limit, events)
                _require(
                    len(raw) == status[prefix + "_bytes"]
                    and hashlib.sha256(raw).hexdigest() == status[prefix + "_sha256"],
                    "declared native artifact differs",
                )
                artifacts[prefix] = raw
        bindings = None
        if original is not None and status["bindings_complete"]:
            bindings = nativeoutput.validate_bindings(
                artifacts["bindings"],
                status,
                request,
                original.contexts,
                original.payload,
            )
            result["bindings"] = {
                "artifact": file_record(work / "bindings.json"),
                "verified": True,
                "scan_kind": bindings["scan"]["kind"],
                "calls": len(bindings["scan"].get("calls", [])),
            }
            phase, path = "receipt", workspace / "bindings-observation.json"
            write_bytes(
                path,
                encode_json(result["bindings"], events=events, path=path),
                events=events,
            )
        if status["status"] in {"self-check-complete", "generation-complete"}:
            required = (
                {"status.json"}
                if original is None
                else {
                    "status.json",
                    "tokens.json",
                    "inferred-config.json",
                    "bindings.json",
                }
            )
            _require(
                set(path.name for path in work.iterdir()) == required,
                "native command emitted unexpected artifacts",
            )
        if status["status"] == "self-check-complete":
            nativeoutput.validate_self_check(status_bytes, request, configuration)
            result["outcome"] = "self-check-complete"
        elif original is not None and status["status"] in {
            "generation-failed",
            "generation-complete",
        }:
            arguments = dict(
                member=original.member,
                image=original.image,
                request=request,
                configuration=configuration,
                status=status_bytes,
                tokens=artifacts.get("tokens", b""),
                inferred=artifacts.get("inferred_configuration", b""),
                payload=original.payload,
                expected_scope=original.scope,
                bindings=bindings,
                contexts=replace(
                    original.contexts, input_directory=workspace / "input"
                ),
                entry_captures=original.entry_observation.files
                if original.entry_observation
                else None,
            )
            if status["status"] == "generation-failed":
                if status["inferred_configuration_complete"]:
                    nativeoutput.validate_generation(**arguments, require_success=False)
            else:
                unit = nativeoutput.decode_unit(**arguments)
                copies = {
                    "generated.c": unit.source,
                    "pcs.json": encode_json(
                        [asdict(span) for span in unit.spans],
                        events=events,
                        path=workspace / "pcs.json",
                    ),
                }
                result["expected_outputs"] = [
                    dict(
                        path=str(workspace / name),
                        bytes=len(data),
                        sha256=hashlib.sha256(data).hexdigest(),
                    )
                    for name, data in copies.items()
                ]
                expected.extend(result["expected_outputs"])
                modes.update(
                    {
                        str(workspace / "generated.c"): 0o400,
                        str(workspace / "pcs.json"): 0o600,
                    }
                )
                for name, data in copies.items():
                    phase, path = "output-copy", workspace / name
                    write_bytes(
                        path, data, readonly=name == "generated.c", events=events
                    )
                recipe.verify_inputs(result["expected_outputs"], events=events)
                result.update(
                    source=result["expected_outputs"][0],
                    pcs=result["expected_outputs"][1],
                    endpoint=unit.endpoint,
                    entry_symbol=unit.entry_symbol,
                    outcome="generated",
                )
    except Exception as error:
        retain(
            events,
            error,
            phase=phase,
            path=path,
            origin="cleanup"
            if isinstance(error, ProcessCleanupError)
            else "policy"
            if isinstance(
                error,
                (
                    PolicyFailure,
                    recipe.ImmutableInputDrift,
                    CaptureProofRefused,
                    nativeoutput.NativeOutputRefused,
                ),
            )
            else "unknown",
        )
    if "process" not in result and result["launch"] == "unknown":
        try:
            result["process"] = _load(
                workspace / "native.json",
                (6 * command.output_limit) + (64 << 10),
                events,
            )
            result["process_origin"] = "persisted"
            result["launch"] = verify_capture(
                result["process"],
                workspace,
                command.argv,
                environment=command.environment,
                deadline=command.deadline,
                output_limit=command.output_limit,
                name="native",
                timeout=command.timeout,
                origin="persisted",
                events=events,
                reader=active_reader(),
            )
        except Exception as error:
            retain(
                events,
                error,
                phase="native-capture-recovery",
                path=workspace / "native.json",
                origin="policy"
                if isinstance(
                    error,
                    (PolicyFailure, recipe.ImmutableInputDrift, CaptureProofRefused),
                )
                else "unknown",
            )
    if result.get("process", {}).get("owner_observation") is not None:
        _owner = primary(events)
        if not isinstance(_owner, ProcessCleanupError):
            retain(
                events,
                PolicyFailure("native capture returned unresolved ownership"),
                phase="owner",
                path=workspace,
                origin="policy",
            )
    events.extend(
        verify_native(
            command,
            original,
            NativeAttempt(unit, result, primary(events)),
            receipt=False,
            source_form=source_form,
        )
    )
    pending = primary(events)
    if pending is not None:
        result.update(
            failure=f"{type(pending).__name__}: {pending}",
            capture_failures=getattr(pending, "capture_failures", ()),
        )
        if isinstance(pending, ProcessCleanupError):
            result["owner_observation"] = pending.observed
    result["secondary_failures"] = facts(events)
    try:
        encoded = encode_json(
            result, events=events, path=workspace / "native-result.json"
        )
        _require(len(encoded) <= 1 << 20, "native result exceeds proof bound")
        write_bytes(workspace / "native-result.json", encoded, events=events)
        events.extend(
            verify_native(
                command,
                original,
                NativeAttempt(unit, result, primary(events)),
                source_form=source_form,
            )
        )
    except Exception as error:
        retain(
            events,
            error,
            phase="receipt",
            path=workspace / "native-result.json",
            origin="policy"
            if isinstance(error, (PolicyFailure, recipe.ImmutableInputDrift))
            else "unknown",
        )
    pending = primary(events)
    if pending is not None:
        result.update(
            failure=f"{type(pending).__name__}: {pending}",
            capture_failures=getattr(pending, "capture_failures", ()),
        )
        if isinstance(pending, ProcessCleanupError):
            result["owner_observation"] = pending.observed
    result["secondary_failures"] = facts(events)
    return NativeAttempt(unit, result, pending, tuple(events))


@dataclass(frozen=True)
class AnswerAidOrigin:
    answer: FileIdentity
    selector: str
    retained_files: tuple[FileIdentity, ...] = ()

    def as_record(self):
        return dict(
            answer=self.answer.as_record(),
            selector=self.selector,
            retained_files=[row.as_record() for row in self.retained_files],
        )


@dataclass(frozen=True)
class EntryPrototype:
    member: provenance.BoundMember
    symbol: str
    text: str
    origin: AnswerAidOrigin

    def as_record(self):
        return dict(
            member=asdict(self.member),
            symbol=self.symbol,
            text=self.text,
            origin=self.origin.as_record(),
        )


@dataclass(frozen=True)
class ExtraFunction:
    prototype: EntryPrototype

    def as_record(self):
        return dict(kind="function", prototype=self.prototype.as_record())


@dataclass(frozen=True)
class ObjectDeclarations:
    document: FileIdentity
    content: bytes
    origin: AnswerAidOrigin

    def as_record(self):
        return dict(
            kind="objects",
            document=self.document.as_record(),
            origin=self.origin.as_record(),
        )


@dataclass(frozen=True)
class SuppliedPlan:
    entry: EntryPrototype
    metadata: tuple[ExtraFunction | ObjectDeclarations, ...]

    def as_record(self):
        return dict(
            entry=self.entry.as_record(),
            metadata=[row.as_record() for row in self.metadata],
        )


@dataclass(frozen=True)
class OrdinaryUnit:
    entry_symbol: str
    source: bytes
    source_file: FileIdentity
    tokens: FileIdentity
    console: FileIdentity


@dataclass(frozen=True)
class FreshSuppliedInspection:
    unit: OrdinaryUnit | None
    authenticated_mode: str | None
    predicates: tuple[PredicateResult, ...]
    files: tuple[FileIdentity, ...]
    raw: dict
    events: tuple[FailureEvent, ...]


@dataclass(frozen=True)
class OriginalOnlySource:
    native: dict
    outputs: dict


def verify_submitted_original_outputs(outputs, limit, events):
    """Authenticate the original producer's retained declaration and binding outputs."""
    documents = {}
    for name in ("status.json", "inferred-config.json", "bindings.json"):
        record = outputs[name]
        _, content = _consume_identity(
            record, active_reader() or physical_read_bytes, events
        )
        documents[name] = nativeoutput.parse_json(content, limit)
    status, declarations, bindings = (
        documents[name]
        for name in ("status.json", "inferred-config.json", "bindings.json")
    )
    _require(
        status.get("inferred_configuration_complete") is True
        and status.get("bindings_complete") is True
        and (
            status.get("inferred_configuration_bytes"),
            status.get("inferred_configuration_sha256"),
        )
        == (
            outputs["inferred-config.json"]["bytes"],
            outputs["inferred-config.json"]["sha256"],
        )
        and (status.get("bindings_bytes"), status.get("bindings_sha256"))
        == (outputs["bindings.json"]["bytes"], outputs["bindings.json"]["sha256"])
        and type(declarations.get("functions")) is list
        and bool(declarations["functions"])
        and all(type(function) is dict for function in declarations["functions"])
        and {"schema", "root", "scan", "request_sha256"} <= bindings.keys(),
        "submitted declarations or bindings lack complete retained identities",
    )


@dataclass(frozen=True)
class SuppliedOrdinarySource:
    inspection: FreshSuppliedInspection


SUPPLIED_KIND = "supplied-ordinary-v1"
SUPPLIED_LIMIT = 64 << 20
CONTEXT_LIMIT = 256 << 10
REVIEWED_SUPPORT_SHA256 = (
    "ca7800b3d0fe51c794ab9dfeace9cb424438955dece8213b67093a55d86db756"
)
REVIEWED_RIZIN_TYPES_SHA256 = (
    "a2e9eddb2170b30f950226918d0672d77cd5893605bb9c2d4887f3e0d69f0803"
)
REVIEWED_RIZIN_TYPE_RECIPE_SHA256 = (
    "c914122cc9bf11806e50b87b60dc61ceba6816e33e6cfa87f42874e1ba76fcb5"
)
REVIEWED_RIZIN_DATA_SHA256 = (
    "5dba871655bff4f7d8dc7974d5ce3157676c6e99cc79a49e8c0b72c0db08bac4"
)
REVIEWED_NATIVE_RECEIPT_SHA256 = (
    "4a167a1f35bc64c2531470881e0e570629d3df8217e61b707f65749ddced57b4"
)
REVIEWED_RETDEC_PARAMETERS_SHA256 = (
    "1e67a71af4b4c8fe7a0952c21aa19632037db4222cfd6b925c076f8446e40dca"
)
REVIEWED_RIZIN_RUNTIME_SHA256 = (
    "288001233a01e71968f9594a622df2596aed6b4e2f301b46e978f17c6bbc8287"
)
REVIEWED_RIZIN_PRINTER_SOURCES_SHA256 = (
    "5d1635d4ab2edaaddb0cf356c63c58f1a43583f2d814c44b694be1fdec303c38"
)
REVIEWED_ORDINARY_COMMAND_SOURCE_SHA256 = (
    "3c31ca06392870a35dc2b6d63f8a99d41a34c8a1b2a4bbaadeeafeeb1cb34c29"
)
_RIZIN_PRINTER_SOURCES = (
    "librz/core/cmd/cmd_type.c",
    "librz/core/ctypes.c",
    "librz/core/canalysis.c",
    "librz/arch/var.c",
    "librz/arch/function.c",
    "librz/include/rz_type.h",
    "librz/util/pj.c",
    "librz/include/rz_util/rz_assert.h",
    "librz/util/assert.c",
    "librz/util/log.c",
)
REVIEWED_GENERIC_ORIGINS = {
    "rizin": "cc06c1dedbe47901faecc7e9825ca957aca81216",
    "retdec_library_types": "c1ecff3c0c9b494e89f918595dc00d2bce336ac2",
    "retdec_ordinals": "8fea90b2f7c3facc589e97330393d443ea37c812",
    "retdec_yara_sources": "3a94c2aba7a51bcc00778c70f9f7a36c9c4edfb0",
}
_ORDINARY_DIRECTORIES = (
    "home",
    "xdg-config",
    "xdg-cache",
    "xdg-data",
    "tmp",
    "cache",
    "state",
)
_STATE_COMMANDS = {
    "functions": "aflj",
    "globals": "avglj",
    "types": "tj",
    "callables": "tfj",
    "calling-conventions": "tccj",
    "flags": "flj",
    "loader": "olj",
    "maps": "omlj",
    "entries": "iej",
    "definitions": "tc",
    "signatures": "afsj @@F",
    "locals": "afvlj @@F",
    "blocks": "afbj @@F",
    "graphs": "agf json @@F",
}


def _identity(path, raw):
    return FileIdentity(path, len(raw), hashlib.sha256(raw).hexdigest())


def _inventory_digest(rows):
    return hashlib.sha256(
        json.dumps(rows, sort_keys=True, separators=(",", ":")).encode()
    ).hexdigest()


def _regular(path):
    _require(
        path.is_absolute()
        and path.resolve() == path
        and not path.is_symlink()
        and path.is_file(),
        f"supplied file must be canonical and regular: {path}",
    )


def _consume_identity(record, reader, events):
    _require(
        type(record) is dict
        and set(record) == {"path", "bytes", "sha256"}
        and type(record["path"]) is str
        and type(record["bytes"]) is int
        and 0 <= record["bytes"] <= SUPPLIED_LIMIT
        and type(record["sha256"]) is str
        and re.fullmatch(r"[0-9a-f]{64}", record["sha256"]),
        "supplied file identity differs",
    )
    path = Path(record["path"])
    _regular(path)
    raw = reader(path, max(1, record["bytes"]), events)
    identity = _identity(path, raw)
    _require(identity.as_record() == record, f"supplied file changed: {path}")
    return identity, raw


def _directory_inventory(directory, reader, events):
    _require(
        directory.is_absolute()
        and directory.resolve() == directory
        and directory.is_dir()
        and not directory.is_symlink(),
        "ordinary directory differs",
    )
    files, directories, total = [], ["."], 0
    for path in sorted(directory.rglob("*")):
        check_deadline()
        _require(not path.is_symlink(), "ordinary tree contains a symlink")
        relative = path.relative_to(directory).as_posix()
        if path.is_dir():
            directories.append(relative)
        else:
            _regular(path)
            raw = reader(path, SUPPLIED_LIMIT, events)
            total += len(raw)
            _require(
                total <= 256 << 20 and len(files) < 8192,
                "ordinary tree exceeds finite inventory",
            )
            files.append({**_identity(path, raw).as_record(), "path": relative})
        _require(len(directories) <= 8192, "ordinary tree exceeds directory bound")
    return dict(root=str(directory), directories=directories, files=files)


def _answer_document(prepared_run, events):
    index = bundles.verify_bundle_references(
        prepared_run,
        "answer",
        io_mode="exact",
        events=events,
        max_bytes=SUPPLIED_LIMIT,
        max_entries=65536,
    )
    _require(
        index["origin"] == "evaluator"
        and index["role"] == "answer"
        and len(index["documents"]) == 1,
        "supplied context requires the active evaluator Answer",
    )
    answer = bundles.resolve_document(
        prepared_run,
        contract.doc_ref("answer", index["documents"][0]["digest"], None),
        owner="answer",
        io_mode="exact",
        events=events,
        max_bytes=SUPPLIED_LIMIT,
    )
    path = prepared_run / "answer" / "answer.json"
    return answer, _identity(path, _read(path, SUPPLIED_LIMIT, events))


def _entry_prototype(value, binding, answer, answer_identity):
    _require(
        type(value) is dict and set(value) == {"member_key", "prototype"},
        "entry prototype request differs",
    )
    member = binding.member(value["member_key"])
    reference = answer["members"][member.key]["reference"]
    text, symbol = value["prototype"], reference["authored_symbol"]
    _require(
        type(text) is str
        and len(text) <= 1024
        and text == reference["authored_signature"]
        and type(symbol) is str
        and re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]{0,127}", symbol)
        and re.fullmatch(r"[A-Za-z0-9_ *(),]+", text)
        and re.search(r"\b" + re.escape(symbol) + r"\s*\(", text)
        and text.count("(") == text.count(")") == 1,
        "prototype is not one safe Answer-associated declaration",
    )
    return EntryPrototype(
        member,
        symbol,
        text,
        AnswerAidOrigin(answer_identity, f"members/{member.key}/reference"),
    )


def _supplied_plans(context_raw, binding, prepared_run, events):
    document = nativeoutput.parse_json(context_raw, CONTEXT_LIMIT)
    _require(
        type(document) is dict
        and set(document) == {"schema", "contexts"}
        and document["schema"] == "supplied-context-v1"
        and type(document["contexts"]) is list
        and 1 <= len(document["contexts"]) <= 118,
        "bounded supplied context document differs",
    )
    answer, identity = _answer_document(prepared_run, events)
    _require(
        set(answer["members"]) == {row.key for row in binding.members},
        "supplied Answer membership differs",
    )
    plans, selected = [], set()
    for value in document["contexts"]:
        _require(
            type(value) is dict
            and set(value) == {"member_key", "prototype", "metadata"}
            and type(value["metadata"]) is list
            and len(value["metadata"]) <= 8,
            "supplied request fields differ",
        )
        entry = _entry_prototype(
            {k: value[k] for k in ("member_key", "prototype")},
            binding,
            answer,
            identity,
        )
        _require(entry.member.key not in selected, "duplicate supplied member context")
        selected.add(entry.member.key)
        metadata, seen = [], {entry.member.key}
        objects = False
        for aid in value["metadata"]:
            _require(type(aid) is dict, "supplied metadata is not typed")
            if aid.get("kind") == "function":
                _require(
                    set(aid) == {"kind", "member_key", "prototype"},
                    "extra function fields differ",
                )
                prototype = _entry_prototype(
                    {k: aid[k] for k in ("member_key", "prototype")},
                    binding,
                    answer,
                    identity,
                )
                _require(
                    prototype.member.key not in seen
                    and prototype.member.image_id == entry.member.image_id
                    and prototype.member.target == entry.member.target,
                    "extra function is duplicate or outside this image",
                )
                seen.add(prototype.member.key)
                metadata.append(ExtraFunction(prototype))
                continue
            _require(
                aid.get("kind") == "objects"
                and set(aid) == {"kind", "member_key", "declaration_index"}
                and aid["member_key"] == entry.member.key
                and type(aid["declaration_index"]) is int
                and not objects,
                "unsupported or duplicate supplied metadata",
            )
            references = answer["members"][entry.member.key]["retained"]["declarations"]
            ordinal = aid["declaration_index"]
            _require(
                0 <= ordinal < len(references),
                "object declaration has no retained Answer origin",
            )
            reference = references[ordinal]
            content = bundles.resolve_file(
                prepared_run,
                reference,
                owner="answer",
                io_mode="exact",
                events=events,
                max_bytes=4 << 20,
            )
            parsed = nativeoutput.parse_json(content, 4 << 20)
            _require(
                type(parsed) is dict
                and set(parsed) == {"globals"}
                and type(parsed["globals"]) is list
                and 1 <= len(parsed["globals"]) <= 1024,
                "retained declaration is not an ordinary object document",
            )
            source = _identity(prepared_run / "answer" / reference["path"], content)
            metadata.append(
                ObjectDeclarations(
                    source,
                    content,
                    AnswerAidOrigin(
                        identity,
                        f"members/{entry.member.key}/retained/declarations/{ordinal}",
                        (source,),
                    ),
                )
            )
            objects = True
        plans.append(SuppliedPlan(entry, tuple(metadata)))
    return tuple(plans), identity


def _loader_image(payload, image, member):
    _require(
        0 < len(payload) == image.payload_size <= 16 << 20
        and image.load_address % 4 == 0
        and image.load_address
        <= member.start
        < member.end
        <= image.load_address + len(payload),
        "full loader payload differs",
    )
    offset = 0x100
    strings = b"\0.text\0.shstrtab\0"
    names_offset = offset + len(payload)
    section_offset = (names_offset + len(strings) + 3) & ~3
    header = struct.pack(
        "<16sHHIIIIIHHHHHH",
        b"\x7fELF\x01\x01\x01" + b"\0" * 9,
        2,
        8,
        1,
        member.start,
        52,
        section_offset,
        0x1000,
        52,
        32,
        1,
        40,
        3,
        2,
    )
    segment = struct.pack(
        "<8I",
        1,
        offset,
        image.load_address,
        image.load_address,
        len(payload),
        len(payload),
        5,
        4,
    )
    sections = b"\0" * 40 + struct.pack(
        "<10I", 1, 1, 6, image.load_address, offset, len(payload), 0, 0, 4, 0
    )
    sections += struct.pack("<10I", 7, 3, 0, 0, names_offset, len(strings), 0, 0, 1, 0)
    return (
        header
        + segment
        + b"\0" * (offset - len(header) - len(segment))
        + payload
        + strings
        + b"\0" * (section_offset - names_offset - len(strings))
        + sections
    )


def _verify_loader(raw, payload, image, member):
    extraction.read_linked_image(raw, limit=32 << 20)
    _require(
        raw == _loader_image(payload, image, member),
        "ordinary loader is not the complete original payload envelope",
    )


def _sdb_contents(raw):
    _require(len(raw) >= 1024, "Rizin database is truncated")
    end = struct.unpack_from("<I", raw)[0]
    _require(1024 <= end <= len(raw), "Rizin database record boundary differs")
    offset, values = 1024, {}
    while offset < end:
        _require(offset + 4 <= end, "Rizin database length header is truncated")
        key_size = raw[offset]
        value_size = int.from_bytes(raw[offset + 1 : offset + 4], "little")
        offset += 4
        _require(
            key_size > 1 and value_size > 0 and offset + key_size + value_size <= end,
            "Rizin database key/value bounds differ",
        )
        key, value = (
            raw[offset : offset + key_size],
            raw[offset + key_size : offset + key_size + value_size],
        )
        _require(
            key[-1:] == value[-1:] == b"\0" and b"\0" not in key[:-1] + value[:-1],
            "Rizin database string encoding differs",
        )
        name = key[:-1].decode("utf-8", "strict")
        _require(name not in values, "Rizin database repeats a key")
        values[name] = value[:-1].decode("utf-8", "strict")
        offset += key_size + value_size
    _require(offset == end, "Rizin database record extent differs")
    return values


def _sdb_source(raw):
    values, at_root = {}, True
    for line in raw.decode("utf-8", "strict").splitlines():
        if line.startswith("/"):
            at_root = line == "/"
            continue
        if not at_root or not line:
            continue
        escaped, separator = False, None
        for index, character in enumerate(line):
            if escaped:
                escaped = False
            elif character == "\\":
                escaped = True
            elif character == "=":
                separator = index
                break
        if separator is None:
            continue
        key, value = (
            re.sub(
                r"\\(.)",
                lambda match: {"n": "\n", "r": "\r", "t": "\t"}.get(match[1], match[1]),
                part,
            )
            for part in (line[:separator], line[separator + 1 :])
        )
        if key and value:
            values[key] = value
    return values


def _rizin_type_tables(tables, recipe):
    _require(
        len(recipe) == 2034
        and hashlib.sha256(recipe).hexdigest() == REVIEWED_RIZIN_TYPE_RECIPE_SHA256,
        "Rizin type composition recipe differs from the reviewed upstream source",
    )
    lists = {}
    for name in ("sdb_types_files", "win_types_extra"):
        matches = re.findall(rb"(?m)^" + name.encode() + rb" = (\[[^\]]*\])", recipe)
        _require(len(matches) == 1, "Rizin type composition list differs")
        rows = ast.literal_eval(matches[0].decode("ascii"))
        _require(
            type(rows) is list
            and all(
                type(row) is str and re.fullmatch(r"[a-z0-9-]+", row) for row in rows
            )
            and len(rows) == len(set(rows)),
            "Rizin type composition list repeats or has an unsupported name",
        )
        lists[name] = rows
    installed = {}
    for name in lists["sdb_types_files"]:
        sources = [name + ".sdb.txt"]
        if name in {"types-windows", "functions-windows"}:
            for extra in lists["win_types_extra"]:
                source = f"{name}_{extra}.sdb.txt"
                if name == "functions-windows" or source in tables:
                    sources.append(source)
        _require(
            all(source in tables for source in sources),
            "Rizin type composition has a missing required source",
        )
        merged = {}
        for source in sources:
            merged.update(tables[source])
        installed[f"types/{name}.sdb"] = merged
    return installed


def _support_sources(backend, reader=None, events=None):
    support = backend / "support"
    names = []
    for boundary in ("ordinals", "types", "yara_patterns/signsrch"):
        inventory = _directory_inventory(
            support / boundary, reader or active_reader() or physical_read_bytes, events
        )
        names.extend(f"{boundary}/{row['path']}" for row in inventory["files"])
    return support, sorted(names)


def _ordinary_defaults(run, reader, events):
    snapshot = PreparedInputs.snapshot_records(run)["retdec-source"]
    parameters = next(
        row for row in snapshot["files"] if row["path"] == "src/config/parameters.cpp"
    )
    _require(
        parameters["sha256"] == REVIEWED_RETDEC_PARAMETERS_SHA256,
        "ordinary parameter serialization source differs from the reviewed source",
    )
    for directory in PreparedInputs.snapshot_roots(snapshot):
        _consume_identity(
            {**parameters, "path": str(directory / parameters["path"])},
            reader,
            events,
        )
    _, raw = _consume_identity(run["binaries"]["configuration"], reader, events)
    _require(
        len(raw) <= 64 << 10, "ordinary default configuration source exceeds its bound"
    )
    match = re.search(rb'DefaultConfigJSON\s*=\s*R"\((.*?)\)";', raw, re.S)
    _require(match is not None, "ordinary default configuration source differs")
    document = nativeoutput.parse_json(match[1], 64 << 10)
    _require(
        set(document) == {"decompParams"},
        "ordinary default configuration shape differs",
    )
    return document["decompParams"]


def _generic_supplied_inputs(run, reader, events):
    generic = run["producer"]["generic"]
    _require(
        type(generic) is dict
        and set(generic)
        == {"support", "rizin_data", "rizin_types", "rizin_header", "origins"}
        and generic["origins"] == REVIEWED_GENERIC_ORIGINS,
        "ordinary generic inputs differ",
    )
    snapshots = PreparedInputs.snapshot_records(run)
    backend = PreparedInputs.snapshot_roots(snapshots["retdec-source"])[-1]
    support, names = _support_sources(backend, reader, events)
    source_rows = []
    for name in names:
        path = support / name
        _regular(path)
        raw = reader(path, SUPPLIED_LIMIT, events)
        if name.endswith(".yara"):
            _require(
                not re.search(rb"^\s*(?:include|import)\s", raw, re.M),
                "YARA source include/import closure is unsupported",
            )
        source_rows.append({**_identity(path, raw).as_record(), "path": name})
    _require(
        _inventory_digest(source_rows) == REVIEWED_SUPPORT_SHA256
        and generic["support"] == source_rows,
        "ordinary support is outside the reviewed generic source inventory",
    )
    installed = Path(generic["rizin_data"])
    snapshot = snapshots["rizin-data"]
    _require(
        Path(snapshot["source"]) == installed, "Rizin data source association differs"
    )
    observed = _directory_inventory(installed, reader, events)
    _require(
        _inventory_digest(observed["files"]) == REVIEWED_RIZIN_DATA_SHA256,
        "Rizin database tree is outside the reviewed generic inventory",
    )
    if "copy" in snapshot:
        retained_data = _directory_inventory(Path(snapshot["copy"]), reader, events)
        _require(
            retained_data["files"] == observed["files"]
            and retained_data["directories"] == observed["directories"],
            "ordinary Rizin data copy differs",
        )
    source = snapshots["rizin-type-origins"]
    for directory in PreparedInputs.snapshot_roots(source):
        _require(
            _directory_inventory(directory, reader, events)["files"] == source["files"],
            "complete Rizin type-origin set changed",
        )
    type_rows = [row for row in source["files"] if row["path"].endswith(".sdb.txt")]
    _require(
        _inventory_digest(type_rows) == REVIEWED_RIZIN_TYPES_SHA256
        and generic["rizin_types"] == type_rows
        and {row["path"] for row in source["files"]}
        == {row["path"] for row in type_rows} | {"meson.build"},
        "Rizin type origins are outside the reviewed source inventory",
    )
    recipe_row = next(row for row in source["files"] if row["path"] == "meson.build")
    type_root = PreparedInputs.snapshot_roots(source)[-1]
    _, type_recipe = _consume_identity(
        {**recipe_row, "path": str(type_root / "meson.build")},
        reader,
        events,
    )
    values, tables = {}, {}
    for row in type_rows:
        path = type_root / row["path"]
        raw = reader(path, SUPPLIED_LIMIT, events)
        _require(
            _identity(path, raw).as_record() == {**row, "path": str(path)},
            "Rizin type origin changed",
        )
        tables[row["path"]] = _sdb_source(raw)
        for key, value in tables[row["path"]].items():
            values.setdefault(key, set()).add(value)
    installed_types = _rizin_type_tables(tables, type_recipe)
    observed_types = [
        row
        for row in observed["files"]
        if row["path"].startswith("types/") and row["path"].endswith(".sdb")
    ]
    _require(
        {row["path"] for row in observed_types} == set(installed_types),
        "installed Rizin type database set differs from the reviewed composition",
    )
    for row in observed_types:
        _require(
            _sdb_contents(reader(installed / row["path"], SUPPLIED_LIMIT, events))
            == installed_types[row["path"]],
            "installed Rizin type database differs from its source table",
        )
    _, header = _consume_identity(generic["rizin_header"], reader, events)
    text = header.decode("utf-8", "strict")
    match = re.search(r'#define\s+RZ_PLUGINS\s+"([A-Za-z0-9_./-]+)"', text)
    _require(
        match is not None
        and "RZ_IS_PORTABLE 0" in text
        and installed
        == Path(run["binaries"]["rizin"]["path"]).parents[1] / "share/rizin",
        "Rizin fixed-prefix/home path association differs",
    )
    return support, names, match[1], _project_generic(installed_types, values)


def verify_supplied_run_inputs(run, destination):
    producer = run.get("producer")
    _require(
        type(producer) is dict
        and set(producer)
        == {"kind", "context", "context_copy", "answer", "plans", "generic"}
        and producer["kind"] == SUPPLIED_KIND
        and "binding_inputs" not in run
        and "source_form" not in run,
        "ordinary supplied producer structure differs",
    )
    events = []
    reader = active_reader() or _read
    _require(
        run["binaries"]["receipt"]["sha256"] == REVIEWED_NATIVE_RECEIPT_SHA256,
        "ordinary producer does not use the reviewed current native receipt",
    )
    for record in run["binaries"].values():
        _consume_identity(record, reader, events)
    _, source = _consume_identity(producer["context"], reader, events)
    copied, content = _consume_identity(producer["context_copy"], reader, events)
    _require(
        copied.path == destination / "contexts.json" and source == content,
        "supplied context source/copy differs",
    )
    prepared_run = Path(run["problem"]["run_dir"])
    binding = provenance.open_problem_binding(
        prepared_run, io_mode="exact", events=events
    )
    plans, answer = _supplied_plans(content, binding, prepared_run, events)
    _require(
        producer["answer"] == answer.as_record()
        and producer["plans"] == [row.as_record() for row in plans]
        and run["selected"] == [row.entry.member.key for row in plans],
        "supplied Answer association or projected aids changed",
    )
    _generic_supplied_inputs(run, reader, events)
    if primary(events) is not None:
        raise primary(events)
    return plans


def _quoted_path(path):
    text = str(path)
    _require(
        re.fullmatch(r"/[A-Za-z0-9_./@+-]+", text) is not None,
        "ordinary command path cannot be rendered safely",
    )
    return '"' + text + '"'


def _state_script(workspace, phase):
    commands = []
    for name, command in _STATE_COMMANDS.items():
        path = workspace / "state" / f"{phase}-{name}.raw"
        commands.append(f"{command} > {_quoted_path(path)}")
    return commands


def _ordinary_command(workspace, run, plan):
    entry = plan.entry
    commands = [
        f"L {_quoted_path(Path(run['binaries']['plugin']['path']))}",
        *_state_script(workspace, "before"),
    ]
    prototypes = [
        entry,
        *(row.prototype for row in plan.metadata if isinstance(row, ExtraFunction)),
    ]
    for prototype in prototypes:
        member = prototype.member
        commands += [
            f"e analysis.from={member.start}",
            f"e analysis.to={member.end}",
            f"s {member.start}",
            f"af {prototype.symbol}",
            f'afs "{prototype.text}"',
        ]
    commands += [
        f"e analysis.from={entry.member.start}",
        f"e analysis.to={entry.member.end}",
        f"s {entry.member.start}",
        *_state_script(workspace, "pre"),
        f"s {entry.member.start}",
        f"pdz > {_quoted_path(workspace / 'ordinary.c')}",
        *_state_script(workspace, "after"),
        "q",
    ]
    environment = {
        "HOME": str(workspace / "home"),
        "XDG_CONFIG_HOME": str(workspace / "xdg-config"),
        "XDG_CACHE_HOME": str(workspace / "xdg-cache"),
        "XDG_DATA_HOME": str(workspace / "xdg-data"),
        "TMPDIR": str(workspace / "tmp"),
        "DEC_SAVE_DIR": str(workspace / "cache"),
        "LC_ALL": "C",
        "PATH": "/usr/bin:/bin",
        "RZ_NOPLUGINS": "1",
    }
    if any(isinstance(row, ObjectDeclarations) for row in plan.metadata):
        environment["DEC_OBJECT_DECLARATIONS"] = str(workspace / "input/objects.json")
    return NativeCommand(
        workspace,
        (
            run["binaries"]["rizin"]["path"],
            "-N",
            "-q",
            "-a",
            "mips",
            "-b",
            "32",
            "-e",
            "scr.color=false",
            "-e",
            "analysis.hasnext=false",
            "-e",
            "analysis.calls=false",
            "-e",
            "analysis.limits=true",
            "-e",
            f"analysis.from={entry.member.start}",
            "-e",
            f"analysis.to={entry.member.end}",
            "-c",
            "; ".join(commands),
            str(workspace / "input/original.elf"),
        ),
        environment,
        run["limits"]["deadline"],
        run["limits"]["process_output_bytes"],
        60.0,
    )


def _json_sequence(raw):
    text = raw.decode("utf-8", "strict")
    decoder, values = json.JSONDecoder(), []
    offset = 0
    while offset < len(text):
        while offset < len(text) and text[offset].isspace():
            offset += 1
        if offset == len(text):
            break
        value, offset = decoder.raw_decode(text, offset)
        values.append(value)
        _require(
            len(values) <= 118, "ordinary per-function state exceeds selected closure"
        )
    return values


def _named_rows(value):
    _require(
        type(value) is list
        and all(type(row) is dict and type(row.get("name")) is str for row in value),
        "ordinary named state listing differs",
    )
    names = {row["name"]: row for row in value}
    _require(len(names) == len(value), "ordinary state listing repeats a name")
    return names


def _signature_census(rows, functions, conventions):
    signatures = _named_rows(rows)
    _require(
        set(signatures) == set(functions),
        "ordinary signature census does not cover every function",
    )
    register_arguments = conventions.get("reg", {}).get("args", [])
    for name, signature in signatures.items():
        _require(
            {"name", "noreturn", "ret", "args"} <= signature.keys()
            and signature.keys() <= {"name", "noreturn", "ret", "args", "cc"}
            and type(signature["noreturn"]) is bool
            and signature["noreturn"] == functions[name].get("noreturn")
            and type(signature["ret"]) is str
            and bool(signature["ret"])
            and signature.get("cc") == functions[name].get("calltype"),
            "ordinary signature schema or function calling convention differs",
        )
        if "cc" in signature:
            _require(
                type(signature["cc"]) is str and signature["cc"] in conventions,
                "ordinary signature calling convention has no observed generic origin",
            )
        arguments = signature["args"]
        _named_rows(arguments)
        for index, argument in enumerate(arguments):
            _require(
                {"name", "type"} <= argument.keys()
                and argument.keys() <= {"name", "type", "cc"}
                and bool(argument["name"])
                and type(argument["type"]) is str
                and bool(argument["type"])
                and argument.get("cc")
                == (
                    register_arguments[index]
                    if index < len(register_arguments)
                    else None
                ),
                "ordinary signature argument schema or register role differs",
            )
    return signatures


def _local_census(rows, function_names):
    _require(
        type(rows) is list and len(rows) == len(function_names),
        "ordinary local-variable census does not cover every function",
    )
    locals_by_function = {}
    for name, buckets in zip(function_names, rows, strict=True):
        _require(
            type(buckets) is dict and buckets.keys() <= {"stack", "reg"},
            "ordinary local-variable storage buckets differ",
        )
        combined = []
        for kind in ("stack", "reg"):
            bucket = buckets.get(kind, [])
            _require(
                type(bucket) is list,
                "ordinary local-variable storage bucket is not an array",
            )
            for local in bucket:
                _require(
                    type(local) is dict
                    and set(local) == {"name", "arg", "type", "storage"}
                    and type(local["name"]) is str
                    and bool(local["name"])
                    and type(local["arg"]) is bool
                    and type(local["type"]) is str
                    and bool(local["type"]),
                    "ordinary local-variable row differs",
                )
                storage = local["storage"]
                _require(
                    type(storage) is dict
                    and set(storage) == {"type", kind}
                    and storage["type"] == kind
                    and (
                        type(storage[kind]) is int
                        if kind == "stack"
                        else type(storage[kind]) is str and bool(storage[kind])
                    ),
                    "ordinary local-variable storage differs from its bucket",
                )
                combined.append(local)
        locals_by_function[name] = _named_rows(combined)
    return locals_by_function


def _type_spelling(text):
    return re.sub(r"\s+", "", text)


@dataclass(frozen=True)
class _ArgumentProjection:
    name: str
    native_type: str


@dataclass(frozen=True)
class _ExplicitVoidParameters:
    @property
    def c_arity(self):
        return 0

    @property
    def rizin_arguments(self):
        return (_ArgumentProjection("arg0", "void"),)

    @property
    def retdec_arguments(self):
        return ()


@dataclass(frozen=True)
class _DeclaredParameters:
    arguments: tuple[_ArgumentProjection, ...]

    @property
    def c_arity(self):
        return len(self.arguments)

    @property
    def rizin_arguments(self):
        return self.arguments

    @property
    def retdec_arguments(self):
        return self.arguments


@dataclass(frozen=True)
class _PrototypeProjection:
    return_type: str
    parameters: _ExplicitVoidParameters | _DeclaredParameters


@dataclass(frozen=True)
class _GenericProjection:
    values: Mapping[str, frozenset[str]]
    base_kinds: Mapping[str, str]
    callables: Mapping[str, dict]


def _project_type_spelling(text, base_kinds):
    match = re.fullmatch(
        r"\s*(const\s+)?(?:(struct|union|enum)\s+)?"
        r"([A-Za-z_]\w*(?:\s+[A-Za-z_]\w*)*)"
        r"((?:\s*\*\s*(?:const\b\s*)?)*)\s*",
        text,
    )
    _require(match is not None, "unsupported ordinary producer type grammar")
    qualifier, tag, base, pointers = match.groups()
    base = " ".join(base.split())
    _require(
        base not in {"const", "struct", "union", "enum"},
        "ordinary producer type has no base name",
    )
    sized = {
        "char",
        "short",
        "short int",
        "int",
        "long",
        "long int",
        "long long",
        "long long int",
    }
    builtins = sized | {"void", "float", "double", "long double", "_Bool"}
    builtins |= {f"{sign} {name}" for sign in ("signed", "unsigned") for name in sized}
    _require(
        " " not in base or base in builtins,
        "unsupported ordinary producer base type grammar",
    )
    kind = base_kinds.get(base)
    _require(
        not tag or kind in {None, tag},
        "ordinary producer tag conflicts with the active base kind",
    )
    kind = tag or kind
    _require(
        not qualifier or kind not in {"union", "enum"},
        "unsupported ordinary producer tagged qualifier",
    )
    prefix = f"{kind} " if kind in {"struct", "union", "enum"} else ""
    const = "const " if qualifier and kind not in {"struct", "typedef"} else ""
    return const + prefix + base + pointers.strip()


def _project_generic(installed, values):
    kinds, active = {}, []
    for name in (
        "atomic",
        "libc",
        "32",
        "mips",
        "mips-32",
        "linux",
        "linux-32",
        "mips-linux",
        "mips-linux-32",
    ):
        table = installed.get(f"types/types-{name}.sdb", {})
        active.append(table)
        kinds.update(
            {
                key: value
                for key, value in table.items()
                if value in {"type", "typedef", "struct", "union", "enum"}
            }
        )
    for table in active:
        for name, kind in table.items():
            if kind == "typedef":
                _project_type_spelling(table.get(f"typedef.{name}", ""), kinds)
            elif kind in {"struct", "union", "enum"}:
                members = table.get(f"{kind}.{name}", "")
                _require(
                    kind != "enum" or bool(members),
                    "ordinary producer enum cases are missing",
                )
                for member in members.split(",") if members else ():
                    value = table.get(f"{kind}.{name}.{member}")
                    _require(
                        value is not None, "ordinary producer base member is missing"
                    )
                    if kind == "enum":
                        _require(
                            re.fullmatch(r"(?:0x)?[0-9a-fA-F]+", value),
                            "unsupported ordinary producer enum value",
                        )
                        continue
                    parts = value.split(",")
                    _require(
                        len(parts) in {2, 3}
                        and re.fullmatch(r"-?[0-9]+", parts[1])
                        and (len(parts) == 2 or re.fullmatch(r"[0-9]+", parts[2])),
                        "unsupported ordinary producer member storage",
                    )
                    _project_type_spelling(parts[0], kinds)
    callables = {}
    for table_name in ("functions-libc", "functions-linux"):
        table = installed.get(f"types/{table_name}.sdb", {})
        for name, kind in table.items():
            if kind != "func":
                continue
            count = table.get(f"func.{name}.args", "0")
            _require(
                re.fullmatch(r"[0-9]+", count) and int(count) <= 128,
                "unsupported ordinary producer callable arity",
            )
            arguments = []
            for index in range(int(count)):
                value = table.get(f"func.{name}.arg.{index}")
                _require(
                    value is not None, "ordinary producer callable argument is missing"
                )
                native_type, separator, argument_name = value.partition(",")
                _require(
                    "," not in argument_name,
                    "unsupported ordinary producer callable argument fields",
                )
                arguments.append(
                    dict(
                        name=argument_name if separator else f"arg{index}",
                        type=_project_type_spelling(native_type, kinds),
                    )
                )
            callables[name] = dict(
                name=name,
                ret=_project_type_spelling(
                    table.get(f"func.{name}.ret", "void"), kinds
                ),
                args=arguments,
            )
    return _GenericProjection(
        MappingProxyType({key: frozenset(value) for key, value in values.items()}),
        MappingProxyType(kinds),
        MappingProxyType(callables),
    )


def _project_prototype(prototype, generic):
    match = re.fullmatch(
        r"(.*?)\b" + re.escape(prototype.symbol) + r"\s*\((.*?)\)\s*", prototype.text
    )
    _require(
        match is not None and prototype.symbol != "entry0",
        "ordinary authored prototype conflicts with loader state or syntax",
    )
    result, clause = match.groups()
    _require(bool(clause.strip()), "unsupported ordinary unspecified parameter clause")
    parameters = _ExplicitVoidParameters()
    if clause.strip() != "void":
        arguments = []
        for argument in clause.split(","):
            parsed = re.fullmatch(
                r"(.+?(?:\s+|\*\s*))([A-Za-z_]\w*)\s*", argument.strip()
            )
            _require(
                parsed is not None, "unsupported ordinary authored argument declarator"
            )
            native_type = _project_type_spelling(parsed[1], generic.base_kinds)
            _require(
                _type_spelling(native_type) != "void",
                "ordinary named void argument has no explicit-void origin",
            )
            arguments.append(_ArgumentProjection(parsed[2], native_type))
        parameters = _DeclaredParameters(tuple(arguments))
    return _PrototypeProjection(
        _project_type_spelling(result, generic.base_kinds), parameters
    )


def _ordinary_llvm_type(text, primitives):
    if text in primitives:
        return primitives[text]
    tokens = re.findall(r"[A-Za-z_][A-Za-z0-9_]*|\*", text)
    tokens = [token for token in tokens if token not in {"const", "unsigned", "signed"}]
    _require(bool(tokens), "ordinary type spelling is empty")
    if any(token not in primitives and token != "*" for token in tokens):
        return "void"
    return "".join(primitives.get(token, token) for token in tokens)


def _ordinary_translation(run, reader, events):
    snapshot = PreparedInputs.snapshot_records(run)["plugin-source"]
    directory = PreparedInputs.snapshot_roots(snapshot)[-1]
    raw = reader(directory / "src/rz-plugin/utils.cpp", 1 << 20, events)
    expected = next(
        row for row in snapshot["files"] if row["path"] == "src/rz-plugin/utils.cpp"
    )
    _require(
        _identity(directory / expected["path"], raw).as_record()
        == {**expected, "path": str(directory / expected["path"])},
        "ordinary primitive conversion source changed",
    )
    match = re.search(rb"FormatUtils::_primitives\s*\{(.*?)\};", raw, re.S)
    _require(match is not None, "ordinary primitive conversion source differs")
    body = re.sub(rb"//[^\n]*", b"", match[1])
    primitives = dict(
        (key.decode(), value.decode())
        for key, value in re.findall(
            rb'\{"([A-Za-z0-9_]+)",\s*"([A-Za-z0-9_]+)"\}', body
        )
    )
    _require(
        primitives.get("void") == "void",
        "ordinary primitive conversion source is incomplete",
    )
    return primitives


def _generic_callable(row, expected):
    _require(
        set(row) == set(expected) == {"name", "ret", "args"}
        and row["name"] == expected["name"]
        and _type_spelling(row["ret"]) == _type_spelling(expected["ret"])
        and type(row["args"]) is list
        and len(row["args"]) == len(expected["args"]),
        "ordinary callable differs from its coherent active source record",
    )
    for argument, origin in zip(row["args"], expected["args"], strict=True):
        _require(
            type(argument) is dict
            and set(argument) == {"name", "type"}
            and argument["name"] == origin["name"]
            and _type_spelling(argument["type"]) == _type_spelling(origin["type"]),
            "ordinary callable argument differs from its active source projection",
        )


def _require_prototype_state(
    prototype,
    generic,
    signature,
    callable_row,
    function,
    observed_locals,
    calltype,
    primitives,
):
    expected = _project_prototype(prototype, generic)
    arguments = expected.parameters.rizin_arguments
    _require(
        _type_spelling(signature["ret"]) == _type_spelling(expected.return_type)
        and len(signature["args"]) == len(arguments),
        "observed supplied signature differs from its native projection",
    )
    for actual, origin in zip(signature["args"], arguments, strict=True):
        _require(
            actual["name"] == origin.name
            and _type_spelling(actual["type"]) == _type_spelling(origin.native_type),
            "observed supplied argument differs from its native projection",
        )
    _require(
        callable_row
        == dict(
            name=prototype.symbol,
            ret=signature["ret"],
            args=[
                {key: value for key, value in argument.items() if key != "cc"}
                for argument in signature["args"]
            ],
        ),
        "ordinary supplied callable differs from the complete observed signature",
    )
    retained = expected.parameters.retdec_arguments
    _require(
        function.get("returnType", {}).get("llvmIr", "")
        == _ordinary_llvm_type(signature["ret"], primitives)
        and len(function.get("parameters", [])) == len(retained),
        "ordinary configured prototype differs",
    )
    _require(
        function.get("fncType") == "userDefined"
        and function.get("callingConvention")
        == ("mips_default" if calltype == "n32" else "unknown"),
        "ordinary configured function origin or calling convention differs",
    )
    if isinstance(expected.parameters, _ExplicitVoidParameters):
        _require(
            "parameters" not in function
            and signature["args"] == [dict(name="arg0", type="void", cc="a0")]
            and generic.values.get("cc.n32.arg0") == {"a0"}
            and calltype == "n32"
            and observed_locals.get("arg0")
            == dict(
                name="arg0", arg=True, type="void", storage=dict(type="reg", reg="a0")
            ),
            "ordinary explicit-void parameter or retained register local differs",
        )
    for index, (parameter, argument) in enumerate(
        zip(function.get("parameters", []), retained, strict=True)
    ):
        observed = signature["args"][index]
        _require(
            parameter.get("name") == parameter.get("realName") == argument.name
            and parameter.get("type", {}).get("cType") == observed["type"]
            and parameter["type"].get("llvmIr")
            == _ordinary_llvm_type(observed["type"], primitives)
            and parameter.get("storage", {"type": "undefined"})
            == {"type": "undefined"},
            "ordinary configured argument differs from its final projection",
        )
    configured_locals = _named_rows(function.get("locals", []))
    _require(
        set(configured_locals) == set(observed_locals),
        "ordinary configured local-variable inventory differs",
    )
    for name, local in observed_locals.items():
        actual, storage = configured_locals[name], local["storage"]
        kind, offset = storage["type"], storage.get("stack")
        converted = (
            dict(type="register", value=storage["reg"])
            if kind == "reg"
            else dict(type="stack", value=offset - 4 if offset > 0 else offset)
        )
        _require(
            actual.get("name") == actual.get("realName") == name
            and actual.get("type", {}).get("llvmIr")
            == _ordinary_llvm_type(local["type"], primitives)
            and actual.get("storage") == converted,
            "ordinary configured local-variable state differs",
        )


def _require_loader_transition(plan, before, current, functions, generic_callables):
    member = plan.entry.member
    entries = before["entries"]
    _require(
        type(entries) is list
        and len(entries) == 1
        and entries[0].get("type") == "program"
        and entries[0].get("vaddr") == member.start
        and "entry0" not in generic_callables
        and "entry0" not in functions,
        "ordinary loader entry or authored-name collision differs",
    )
    previous_flags, current_flags = (
        _named_rows(before["flags"]),
        _named_rows(current["flags"]),
    )
    initial = dict(name="entry0", realname="entry0", offset=member.start, size=1)
    _require(
        previous_flags.get("entry0") == initial, "ordinary initial loader flag differs"
    )
    expected_flags = {
        **previous_flags,
        "entry0": {**initial, "size": member.end - member.start},
    }
    for name in set(current_flags) - set(previous_flags):
        _require(name in functions, "ordinary flags include undeclared authored state")
        function = functions[name]
        expected_flags[name] = dict(
            name=name, realname=name, offset=function["offset"], size=function["size"]
        )
    _require(
        current_flags == expected_flags,
        "ordinary flags differ from the exact loader/function transition",
    )
    return {"entry0": dict(name="entry0", args=[])}


def _inspect_state(workspace, plan, config, generic_types, primitives, reader, events):
    generic, generic_types = generic_types, generic_types.values
    states = {}
    for phase in ("before", "pre", "after"):
        value = {}
        for name in _STATE_COMMANDS:
            raw = reader(workspace / "state" / f"{phase}-{name}.raw", 16 << 20, events)
            if name == "definitions":
                value[name] = raw.decode("utf-8", "strict")
            elif name in {"signatures", "locals", "blocks", "graphs"}:
                value[name] = _json_sequence(raw)
            else:
                value[name] = nativeoutput.parse_json(raw, 16 << 20)
        states[phase] = value
    before, current, after = (states[name] for name in ("before", "pre", "after"))
    _require(
        before["functions"] == []
        and before["globals"] == []
        and current["globals"] == []
        and after["globals"] == [],
        "ordinary invocation imported unexplained functions or globals",
    )
    for name in (
        "types",
        "calling-conventions",
        "definitions",
        "loader",
        "maps",
        "entries",
    ):
        _require(
            before[name] == current[name] == after[name],
            f"ordinary {name} state changed outside declared aids",
        )
    for name in (
        "functions",
        "globals",
        "callables",
        "flags",
        "signatures",
        "locals",
        "blocks",
        "graphs",
    ):
        _require(
            current[name] == after[name],
            f"ordinary decompilation changed imported {name} state",
        )
    generic_callables = _named_rows(before["callables"])
    _require(
        set(generic_callables) == set(generic.callables),
        "ordinary callable seed has no generic source origin",
    )
    for row in generic_callables.values():
        _generic_callable(row, generic.callables[row["name"]])
    atomic = {row["type"]: row for row in before["types"]}
    _require(
        len(atomic) == len(before["types"]) and set(atomic) <= set(generic_types),
        "ordinary type seed has no generic source origin",
    )
    for name, row in atomic.items():
        _require(
            ("type" if row.get("kind") == "atomic" else row.get("kind"))
            in generic_types[name],
            "ordinary type kind has no generic source origin",
        )
        if row["kind"] == "atomic":
            _require(
                str(row.get("size")) in generic_types.get(f"type.{name}.size", set()),
                "ordinary atomic width has no generic source origin",
            )
    for name, row in _named_rows(before["calling-conventions"]).items():
        if name == "reg":
            _require(
                row
                == dict(
                    name="reg",
                    ret=next(iter(generic_types["cc.o32.ret"])),
                    args=[
                        next(iter(generic_types[f"cc.o32.arg{index}"]))
                        for index in range(4)
                    ],
                ),
                "Rizin's derived register convention differs from the reviewed MIPS register ABI",
            )
            continue
        _require(
            "cc" in generic_types.get(name, set()),
            "ordinary calling convention has no generic source origin",
        )
        for field in ("ret", "self", "error"):
            if field in row:
                _require(
                    row[field] in generic_types.get(f"cc.{name}.{field}", set()),
                    "ordinary calling convention register differs",
                )
        _require(
            type(row.get("args")) is list
            and str(len(row["args"])) in generic_types.get(f"cc.{name}.maxargs", set()),
            "ordinary calling convention argument count differs",
        )
        for index, value in enumerate(row["args"]):
            _require(
                value in generic_types.get(f"cc.{name}.arg{index}", set()),
                "ordinary calling convention argument register differs",
            )
    prototypes = [
        plan.entry,
        *(row.prototype for row in plan.metadata if isinstance(row, ExtraFunction)),
    ]
    functions = _named_rows(current["functions"])
    _require(
        set(functions) == {row.symbol for row in prototypes},
        "actual ordinary function inventory differs from declared aids",
    )
    for row in prototypes:
        function = functions[row.symbol]
        _require(
            function.get("offset") == row.member.start
            and function.get("size") == row.member.end - row.member.start,
            "actual ordinary function range differs from the complete admitted range",
        )
    callables = _named_rows(current["callables"])
    administrative = _require_loader_transition(
        plan, before, current, functions, generic_callables
    )
    _require(
        set(callables)
        == set(generic_callables)
        | {row.symbol for row in prototypes}
        | set(administrative)
        and all(callables[name] == value for name, value in administrative.items())
        and all(
            callables[name] == value
            for name, value in generic_callables.items()
            if name not in functions
        ),
        "ordinary callable state has an undeclared aid",
    )
    signatures = _signature_census(
        current["signatures"], functions, _named_rows(before["calling-conventions"])
    )
    for name in ("locals", "blocks", "graphs"):
        _require(
            len(current[name]) == len(functions) and before[name] == [],
            "ordinary per-function state census is incomplete",
        )
    observed_locals_by_function = _local_census(current["locals"], tuple(signatures))
    _require(
        type(config.get("functions")) is list,
        "ordinary configuration has no observed functions",
    )
    configured = _named_rows(config["functions"])
    _require(
        set(configured) == set(functions),
        "ordinary configuration imported unexplained functions",
    )
    for row in prototypes:
        function = configured[row.symbol]
        _require(
            int(str(function.get("startAddr")), 0) == row.member.start
            and int(str(function.get("endAddr")), 0) == row.member.end,
            "ordinary configured range differs",
        )
        _require_prototype_state(
            row,
            generic,
            signatures[row.symbol],
            callables[row.symbol],
            function,
            observed_locals_by_function[row.symbol],
            functions[row.symbol].get("calltype"),
            primitives,
        )
    objects = [row for row in plan.metadata if isinstance(row, ObjectDeclarations)]
    requested_globals = (
        []
        if not objects
        else nativeoutput.parse_json(objects[0].content, 4 << 20)["globals"]
    )
    _require(
        config.get("globals", []) == requested_globals,
        "ordinary configuration object declarations differ or were rejected",
    )
    return states


def _ordinary_source(workspace, plan, run, reader, events):
    command = _ordinary_command(workspace, run, plan)
    process = nativeoutput.parse_json(
        reader(workspace / "ordinary.json", 64 << 10, events), 64 << 10
    )
    observed = verify_capture(
        process,
        workspace,
        command.argv,
        environment=command.environment,
        deadline=command.deadline,
        output_limit=command.output_limit,
        name="ordinary",
        timeout=command.timeout,
        origin="persisted",
        events=events,
        reader=reader,
    )
    _require(
        observed == "observed"
        and process.get("raw_exit") == 0
        and process.get("failure") is None
        and process.get("owner_observation") is None,
        "ordinary native capture was not successful and complete",
    )
    configs = sorted((workspace / "cache").rglob("rd_config.json"))
    _require(len(configs) == 1, "ordinary cache has no unique selected configuration")
    folder = configs[0].parent
    config = nativeoutput.parse_json(
        reader(configs[0], nativeoutput.CONFIG_LIMIT, events), nativeoutput.CONFIG_LIMIT
    )
    params = config["decompParams"]
    expected_paths = {
        "inputFile": workspace / "input/original.elf",
        "outputConfigFile": configs[0],
        "outputFile": folder / "rd_dec.json",
        "logFile": folder / "rd_out.log",
        "errFile": folder / "rd_err.log",
    }
    _require(
        all(params.get(key) == str(path) for key, path in expected_paths.items()),
        "ordinary configuration paths escape actual capture",
    )
    _require(
        params.get("selectedRanges")
        == [
            dict(
                start=f"0x{plan.entry.member.start:x}",
                end=f"0x{plan.entry.member.end:x}",
            )
        ]
        and params.get("selectedDecodeOnly") is True
        and params.get("outputFormat") == "json-human",
        "ordinary selected range/format differs",
    )
    defaults = _ordinary_defaults(run, reader, events)
    _, _, plugin_home, types = _generic_supplied_inputs(run, reader, events)
    support = workspace / "home" / plugin_home / "support/retdec"
    _require(
        Path(params.get("ordinalNumDirectory", "")).resolve() == support / "ordinals"
        and [Path(path).resolve() for path in params.get("libraryTypeInfoPaths", [])]
        == [
            support / f"types/{name}.json"
            for name in ("arm", "cstdlib", "linux", "windows", "windrivers")
        ]
        and [Path(path).resolve() for path in params.get("cryptoPatternPaths", [])]
        == [
            support / f"yara_patterns/signsrch/{name}.yarac"
            for name in ("signsrch", "signsrch_regex")
        ],
        "ordinary backend support paths differ from delivered inventory",
    )
    changed = {
        "verboseOut",
        "outputFormat",
        "selectedDecodeOnly",
        "ordinalNumDirectory",
        "libraryTypeInfoPaths",
        "cryptoPatternPaths",
    }
    for key, value in defaults.items():
        if key == "backendAggressiveOpts":
            _require(
                value is False and key not in params,
                "ordinary retired backendAggressiveOpts input differs",
            )
            continue
        if key in changed:
            continue
        _require(
            params.get(key, value if value in ([], "") else None) == value,
            f"ordinary default setting differs: {key}",
        )
    _require(
        params.get("verboseOut") is True, "ordinary verbose capture setting differs"
    )
    states = _inspect_state(
        workspace,
        plan,
        config,
        types,
        _ordinary_translation(run, reader, events),
        reader,
        events,
    )
    tokens_path = folder / "rd_dec.json"
    tokens_raw = reader(tokens_path, nativeoutput.TOKEN_LIMIT, events)
    document = nativeoutput.parse_json(tokens_raw, nativeoutput.TOKEN_LIMIT)
    _require(
        type(document) is dict
        and type(document.get("tokens")) is list
        and 1 <= len(document["tokens"]) <= 262144,
        "ordinary token document differs",
    )
    pieces = []
    for token in document["tokens"]:
        _require(type(token) is dict, "ordinary token is not an object")
        if "addr" in token:
            _require(
                type(token["addr"]) is str
                and (
                    not token["addr"]
                    or re.fullmatch(r"(?:0x)?[0-9A-Fa-f]+", token["addr"])
                ),
                "ordinary annotation address differs",
            )
            continue
        _require(
            type(token.get("kind")) is str and type(token.get("val")) is str,
            "ordinary text token differs",
        )
        pieces.append(token["val"])
    source = "".join(pieces).encode("utf-8", "strict")
    console_path = workspace / "ordinary.c"
    console = reader(console_path, nativeoutput.SOURCE_LIMIT, events)
    _require(
        bool(source) and len(source) <= nativeoutput.SOURCE_LIMIT and source == console,
        "ordinary console and complete unedited token source differ",
    )
    prefix = Path(run["binaries"]["rizin"]["path"]).parents[1]
    runtime = []
    inputs = run["native_build_inputs"]
    for original in inputs["files"]:
        path = Path(original["path"])
        if not path.is_relative_to(prefix):
            continue
        relative = path.relative_to(prefix)
        if relative.parts[0] not in {"bin", "lib"}:
            continue
        identity, _ = _consume_identity(original, reader, events)
        runtime.append({**identity.as_record(), "path": str(relative)})
    _require(
        len(runtime) == 26
        and _inventory_digest(runtime) == REVIEWED_RIZIN_RUNTIME_SHA256
        and run["binaries"]["rizin"] in inputs["files"],
        "ordinary Rizin runtime inventory differs from the reviewed producer",
    )
    snapshot = PreparedInputs.snapshot_records(run)["rizin-type-origins"]
    rizin_source = Path(snapshot["source"]).parents[2]
    sources = []
    for relative in _RIZIN_PRINTER_SOURCES:
        path = rizin_source / relative
        _regular(path)
        raw = reader(path, 1 << 20, events)
        sources.append({**_identity(path, raw).as_record(), "path": relative})
    _require(
        _inventory_digest(sources) == REVIEWED_RIZIN_PRINTER_SOURCES_SHA256,
        "ordinary Rizin printer sources differ from the reviewed producer",
    )
    path = Path(__file__).resolve()
    _regular(path)
    implementation = reader(path, 1 << 20, events).decode("utf-8", "strict")
    commands = []
    for node in ast.parse(implementation).body:
        name = getattr(node, "name", None)
        if (
            isinstance(node, ast.Assign)
            and len(node.targets) == 1
            and isinstance(node.targets[0], ast.Name)
        ):
            name = node.targets[0].id
        if name not in {
            "_STATE_COMMANDS",
            "_quoted_path",
            "_state_script",
            "_ordinary_command",
        }:
            continue
        commands.append(
            dict(name=name, source=ast.get_source_segment(implementation, node))
        )
        if name == "_STATE_COMMANDS":
            _require(
                ast.literal_eval(node.value) == _STATE_COMMANDS,
                "ordinary live state commands differ from their source",
            )
    _require(
        _inventory_digest(commands) == REVIEWED_ORDINARY_COMMAND_SOURCE_SHA256,
        "ordinary command sources differ from the reviewed serializer schedule",
    )
    nullable = tuple(
        tuple(row["name"] for row in states[phase]["callables"] if "ret" not in row)
        for phase in ("before", "pre", "after")
    )
    _require(
        nullable == ((), ("entry0",), ("entry0",)),
        "ordinary nullable callable visits differ from the source-derived printer defect",
    )
    _require(
        reader(folder / "rd_err.log", 16 << 20, events) == b"",
        "ordinary backend diagnostics are not empty",
    )
    warning = (
        b"WARNING: pj_ks: assertion 'j && k && v' failed (line 186); returning j\n"
    )
    _require(
        reader(workspace / "ordinary.stderr.bin", 16 << 20, events)
        == warning * sum(map(len, nullable)),
        "ordinary stderr differs from the source-derived printer defect",
    )
    return (
        source,
        _identity(tokens_path, tokens_raw),
        _identity(console_path, console),
        config,
        states,
    )


def inspect_supplied_attempt(candidate, result, run, binding, prepared_run, reader):
    events, files, unit, mode, raw = [], [], None, None, {}

    def consume(path, bound, local=None):
        content = reader(path, bound, local)
        _require(len(content) <= bound, "ordinary consuming read exceeds bound")
        files.append(_identity(path, content))
        return content

    try:
        with (
            using_reader(consume),
            contract.bounded_reads(SUPPLIED_LIMIT, reader=consume),
        ):
            _require(
                run.get("producer", {}).get("kind") == SUPPLIED_KIND
                and "native" not in result
                and "build" not in result,
                "ordinary supplied attempt carries foreign producer evidence",
            )
            plans = verify_supplied_run_inputs(run, candidate.parent)
            member = binding.member(result["identity"]["key"])
            index = run["selected"].index(member.key)
            plan = plans[index]
            _require(
                candidate == candidate.parent / f"member-{index:03d}"
                and result["identity"] == asdict(member)
                and prepared_run == Path(run["problem"]["run_dir"]),
                "ordinary candidate/member/run association differs",
            )
            supplied = result["supplied"]
            _require(
                type(supplied) is dict
                and supplied.get("workspace") == str(candidate)
                and supplied.get("plan") == plan.as_record(),
                "ordinary capture plan differs",
            )
            command = _ordinary_command(candidate, run, plan)
            _require(
                supplied.get("command")
                == dict(
                    argv=list(command.argv),
                    environment=dict(command.environment),
                    deadline=command.deadline,
                    output_limit=command.output_limit,
                    timeout=command.timeout,
                ),
                "ordinary invocation differs",
            )
            process = supplied["process"]
            observed = verify_capture(
                process,
                candidate,
                command.argv,
                environment=command.environment,
                deadline=command.deadline,
                output_limit=command.output_limit,
                name="ordinary",
                timeout=command.timeout,
                origin="persisted",
                events=events,
                reader=consume,
            )
            _require(
                observed == "observed"
                and process.get("raw_exit") == 0
                and process.get("failure") is None
                and process.get("owner_observation") is None,
                "ordinary native capture was not successful and complete",
            )
            from harness.reconstruction import nativegeneration

            image, original, payload = nativegeneration._original(
                binding, prepared_run, member, events
            )
            for name, expected in (
                ("original.image", original),
                ("payload.raw", payload),
            ):
                _require(
                    consume(candidate / "input" / name, 32 << 20, events) == expected,
                    "ordinary original input differs",
                )
            _verify_loader(
                consume(candidate / "input/original.elf", 32 << 20, events),
                payload,
                image,
                member,
            )
            support_root, names, plugin_home, _ = _generic_supplied_inputs(
                run, consume, events
            )
            delivered = candidate / "home" / plugin_home / "support/retdec"
            support_rows = []
            for name in names:
                origin = support_root / name
                destination = delivered / (
                    name[:-5] + ".yarac" if name.endswith(".yara") else name
                )
                source = consume(origin, SUPPLIED_LIMIT, events)
                copied = consume(destination, SUPPLIED_LIMIT, events)
                _require(
                    source == copied,
                    "ordinary delivered support differs from reviewed source",
                )
                support_rows.append(
                    dict(
                        source=_identity(origin, source).as_record(),
                        delivered=_identity(destination, copied).as_record(),
                        role="crypto-rules"
                        if name.endswith(".yara")
                        else "ordinals"
                        if name.startswith("ordinals/")
                        else "library-types",
                        encoding="yara-source-text"
                        if name.endswith(".yara")
                        else "json"
                        if name.endswith(".json")
                        else "ordinary-file",
                    )
                )
            _require(
                supplied.get("support") == support_rows,
                "ordinary support source/delivery association differs",
            )
            current = {
                name: _directory_inventory(candidate / name, consume, events)
                for name in _ORDINARY_DIRECTORIES
            }
            _require(
                supplied.get("final_directories") == current,
                "ordinary complete final directory census changed",
            )
            initial = supplied.get("initial_directories")
            _require(
                type(initial) is dict
                and set(initial) == set(_ORDINARY_DIRECTORIES)
                and initial["home"] == current["home"],
                "ordinary initial support census differs",
            )
            for name in _ORDINARY_DIRECTORIES[1:]:
                _require(
                    initial[name]
                    == dict(root=str(candidate / name), directories=["."], files=[]),
                    "ordinary initial cache/config/state was not empty",
                )
            expected_input = {"original.image", "payload.raw", "original.elf"}
            objects = [
                row for row in plan.metadata if isinstance(row, ObjectDeclarations)
            ]
            if objects:
                expected_input.add("objects.json")
                _require(
                    consume(candidate / "input/objects.json", 4 << 20, events)
                    == objects[0].content,
                    "ordinary object aid delivery differs",
                )
            inputs = _directory_inventory(candidate / "input", consume, events)
            _require(
                inputs == supplied.get("inputs")
                and {row["path"] for row in inputs["files"]} == expected_input
                and inputs["directories"] == ["."],
                "ordinary complete input closure differs",
            )
            source, tokens, console, config, states = _ordinary_source(
                candidate, plan, run, consume, events
            )
            generated = _identity(
                candidate / "generated.c",
                consume(candidate / "generated.c", nativeoutput.SOURCE_LIMIT, events),
            )
            _require(
                generated.as_record() == result.get("source")
                and source
                == consume(generated.path, nativeoutput.SOURCE_LIMIT, events),
                "ordinary generated C changed from complete tokens",
            )
            header, content = _consume_identity(result["prelude"], consume, events)
            _require(
                header.path == candidate / prelude.PRELUDE_PATH
                and content == prelude.STDINT_PRELUDE
                and result.get("compilation") == "unattempted",
                "ordinary source has no unchanged uniform prelude or invents compiler evidence",
            )
            _require(
                supplied.get("outputs")
                == [
                    generated.as_record(),
                    tokens.as_record(),
                    console.as_record(),
                    header.as_record(),
                ],
                "ordinary extracted output identities differ",
            )
            expected_profiles = [
                (row["profile_id"], row["identity"], row["ordinal"])
                for row in run["tools"]["profiles"]
            ]
            attempts = result.get("profile_attempts")
            _require(
                type(attempts) is list
                and [
                    (
                        row.get("profile_id"),
                        row.get("profile_identity"),
                        row.get("ordinal"),
                    )
                    for row in attempts
                ]
                == expected_profiles
                and all(
                    row.get("disposition") == "unattempted"
                    and row.get("stages") == []
                    and row.get("raw_exit") is None
                    for row in attempts
                ),
                "ordinary producer compiler inventory differs",
            )
            mode = "metadata_assisted" if plan.metadata else "signature_only"
            unit = OrdinaryUnit(plan.entry.symbol, source, generated, tokens, console)
            raw = dict(
                supplied=supplied,
                configuration=config,
                state=states,
                plan=plan.as_record(),
                crypto_support=dict(
                    status="unknown",
                    reason="exact raw YARA source delivery is authenticated; process exit does not prove rule loading or crypto support",
                ),
                files=[row.as_record() for row in dict.fromkeys(files)],
            )
            if primary(events) is not None:
                raise primary(events)
    except Exception as error:
        unit, mode = None, None
        retain(events, error, phase="ordinary-source-inspection", path=candidate)
    return FreshSuppliedInspection(
        unit,
        mode,
        (
            PredicateResult(
                "generation",
                "matched" if unit is not None else "failed",
                "genuine ordinary capture and unchanged full token/console/source equality; source-bound tfj printer defect retained, phase attribution inferred"
                if unit is not None
                else "genuine ordinary capture and unchanged full token/console/source equality",
            ),
            PredicateResult(
                "mode",
                "matched" if mode else "unknown",
                "complete Answer-associated delivered aid, generic support, Rizin state and cache closure",
            ),
        ),
        tuple(dict.fromkeys(files)),
        raw,
        tuple(events),
    )


def supplied_submission(candidate, result, run):
    prepared_run = Path(run["problem"]["run_dir"])
    PreparedInputs(MappingProxyType(run), candidate.parent).verify()
    binding = provenance.open_problem_binding(prepared_run, io_mode="exact")
    inspection = inspect_supplied_attempt(
        candidate,
        result,
        run,
        binding,
        prepared_run,
        active_reader() or physical_read_bytes,
    )
    _require(
        inspection.unit is not None and not inspection.events,
        "ordinary supplied submission cannot be independently reproduced",
    )
    return inspection


def capture_supplied_run(
    *,
    root,
    prepared_run,
    destination,
    native_receipt,
    rizin,
    context,
    work_seconds=120.0,
    output_limit=16 << 20,
):
    root, prepared_run, destination, native_receipt, rizin, context = (
        Path(path).resolve()
        for path in (root, prepared_run, destination, native_receipt, rizin, context)
    )
    _require(
        type(work_seconds) in {int, float}
        and math.isfinite(work_seconds)
        and 0 < work_seconds <= 600,
        "ordinary work duration must be finite and at most 600 seconds",
    )
    _require(
        type(output_limit) is int and 0 < output_limit <= SUPPLIED_LIMIT,
        "ordinary raw output bound differs",
    )
    events = []
    cutoff = resolve_deadline(time.monotonic() + work_seconds)
    with use_deadline(cutoff):
        binding = provenance.open_problem_binding(
            prepared_run, io_mode="exact", events=events
        )
        _require(
            len(binding.members) == 118,
            "ordinary run must preserve all 118 admitted members",
        )
        _regular(context)
        content = _read(context, CONTEXT_LIMIT, events)
        plans, answer = _supplied_plans(content, binding, prepared_run, events)
        _require(
            not destination.exists()
            and destination.parent.is_dir()
            and not destination.is_symlink()
            and not any(
                destination == path
                or destination.is_relative_to(path)
                or path.is_relative_to(destination)
                for path in (prepared_run, context)
            ),
            "ordinary destination overlaps immutable inputs or already exists",
        )
        destination.mkdir(mode=0o700)
        selected = [row.entry.member.key for row in plans]
        report = dict(
            operation="capture-supplied",
            status="failed",
            exit=1,
            denominator=118,
            run_id=binding.run_id,
            binding_digest=binding.digest,
            destination=str(destination),
            selected=selected,
            deadline=cutoff,
            semantic_admission=False,
            autonomous_exact=False,
            failures=[],
            members=[
                dict(
                    identity=asdict(member),
                    generation="unattempted",
                    compilation="unattempted",
                )
                for member in binding.members
            ],
        )
        try:
            check_deadline()
            selection, _ = catalog.stage(
                root,
                destination / "tools",
                all_generic=True,
                deadline=cutoff,
                events=events,
            )
            record = {}
            prepared = PreparedInputs.prepare(
                record,
                destination=destination,
                native_receipt=native_receipt,
                configuration=root
                / "third_party/rz-psxdec/src/rz-plugin/decompiler-config.h",
                rizin=rizin,
                selection=selection,
                driver_root=Path(__file__).resolve().parents[1],
            )
            check_deadline()
            for row in report["members"]:
                row["profile_attempts"] = [
                    dict(
                        profile_id=profile["profile_id"],
                        profile_identity=profile["identity"],
                        ordinal=profile["ordinal"],
                        disposition="unattempted",
                        terminal=False,
                        status="evaluator-unattempted"
                        if row["identity"]["key"] in selected
                        else "not-selected",
                        raw_exit=None,
                        exit_code=None,
                        failure=None,
                        stages=[],
                    )
                    for profile in selection["profiles"]
                ]
            for source, name in (
                (rizin.parents[1] / "share/rizin", "rizin-data"),
                (root / "third_party/rizin/librz/arch/types", "rizin-type-origins"),
            ):
                _directory_inventory(source, _read, events)
                record["snapshots"].append(PreparedInputs.snapshot(source, name))
            snapshots = PreparedInputs.snapshot_records(record)
            support_root, names = _support_sources(
                Path(snapshots["retdec-source"]["source"])
            )
            support = [
                {**file_record(support_root / name), "path": name} for name in names
            ]
            write_bytes(
                destination / "contexts.json", content, readonly=True, events=events
            )
            record.update(
                problem=dict(
                    run_dir=str(prepared_run),
                    binding_digest=binding.digest,
                    problem_sha256=binding.problem_doc_sha256,
                ),
                selected=selected,
                limits=dict(
                    work_seconds=work_seconds,
                    deadline=cutoff,
                    process_output_bytes=output_limit,
                    artifact_bytes=SUPPLIED_LIMIT,
                ),
                producer=dict(
                    kind=SUPPLIED_KIND,
                    context=_identity(context, content).as_record(),
                    context_copy=file_record(destination / "contexts.json"),
                    answer=answer.as_record(),
                    plans=[row.as_record() for row in plans],
                    generic=dict(
                        origins=REVIEWED_GENERIC_ORIGINS.copy(),
                        support=support,
                        rizin_data=str(rizin.parents[1] / "share/rizin"),
                        rizin_types=[
                            row
                            for row in snapshots["rizin-type-origins"]["files"]
                            if row["path"].endswith(".sdb.txt")
                        ],
                        rizin_header=file_record(
                            rizin.parents[1] / "include/librz/rz_userconf.h"
                        ),
                    ),
                ),
            )
            write_bytes(
                destination / "run.json",
                encode_json(record, path=destination, events=events),
                events=events,
            )
            with use_deadline(cutoff):
                prepared.verify(events=events)
                for index, plan in enumerate(plans):
                    check_deadline()
                    workspace = destination / f"member-{index:03d}"
                    row = next(
                        row
                        for row in report["members"]
                        if row["identity"]["key"] == plan.entry.member.key
                    )
                    _capture_supplied_member(
                        workspace, record, plan, binding, prepared_run, row, events
                    )
                prepared.verify(events=events)
                provenance.require_active_binding(
                    binding, prepared_run, io_mode="exact", events=events
                )
                check_deadline()
            report.update(
                status="captured-unverified", exit=0, snapshots_unchanged=True
            )
        except Exception as error:
            retain(events, error, phase="ordinary-run", path=destination)
        report["secondary_failures"] = facts(events)
        report["failures"] = [
            f"{type(error).__name__}: {error}" for _, _, _, error in events
        ]
        if events:
            report.update(status="failed", exit=1)
        write_bytes(
            destination / "report.json",
            encode_json(report, path=destination, events=events),
            events=events,
        )
        if isinstance(primary(events), ProcessCleanupError):
            raise primary(events)
        return report


def _capture_supplied_member(
    workspace, run, plan, binding, prepared_run, row, run_events
):
    from harness.reconstruction import nativegeneration

    events = []
    row["supplied"] = supplied = dict(workspace=str(workspace), plan=plan.as_record())
    try:
        workspace.mkdir(mode=0o700)
        for name in (*_ORDINARY_DIRECTORIES, "input"):
            (workspace / name).mkdir(mode=0o700)
        image, original, payload = nativegeneration._original(
            binding, prepared_run, plan.entry.member, events
        )
        for name, raw in (
            ("original.image", original),
            ("payload.raw", payload),
            ("original.elf", _loader_image(payload, image, plan.entry.member)),
        ):
            write_bytes(workspace / "input" / name, raw, readonly=True, events=events)
        support_root, names, plugin_home, _ = _generic_supplied_inputs(
            run, _read, events
        )
        delivered = workspace / "home" / plugin_home / "support/retdec"
        supplied["support"] = []
        for name in names:
            source = support_root / name
            destination = delivered / (
                name[:-5] + ".yarac" if name.endswith(".yara") else name
            )
            raw = _read(source, SUPPLIED_LIMIT, events)
            write_bytes(destination, raw, readonly=True, events=events)
            supplied["support"].append(
                dict(
                    source=file_record(source),
                    delivered=file_record(destination),
                    role="crypto-rules"
                    if name.endswith(".yara")
                    else "ordinals"
                    if name.startswith("ordinals/")
                    else "library-types",
                    encoding="yara-source-text"
                    if name.endswith(".yara")
                    else "json"
                    if name.endswith(".json")
                    else "ordinary-file",
                )
            )
        for aid in plan.metadata:
            if isinstance(aid, ObjectDeclarations):
                write_bytes(
                    workspace / "input/objects.json",
                    aid.content,
                    readonly=True,
                    events=events,
                )
        supplied["inputs"] = _directory_inventory(workspace / "input", _read, events)
        supplied["initial_directories"] = {
            name: _directory_inventory(workspace / name, _read, events)
            for name in _ORDINARY_DIRECTORIES
        }
        command = _ordinary_command(workspace, run, plan)
        supplied["command"] = dict(
            argv=list(command.argv),
            environment=dict(command.environment),
            deadline=command.deadline,
            output_limit=command.output_limit,
            timeout=command.timeout,
        )
        supplied["process"] = capture(
            workspace,
            list(command.argv),
            environment=command.environment,
            deadline=command.deadline,
            output_limit=command.output_limit,
            name="ordinary",
            timeout=command.timeout,
        )
        source, tokens, console, _, _ = _ordinary_source(
            workspace, plan, run, _read, events
        )
        write_bytes(workspace / "generated.c", source, readonly=True, events=events)
        write_bytes(
            workspace / prelude.PRELUDE_PATH,
            prelude.STDINT_PRELUDE,
            readonly=True,
            events=events,
        )
        row.update(
            source=file_record(workspace / "generated.c"),
            prelude=file_record(workspace / prelude.PRELUDE_PATH),
        )
        supplied["outputs"] = [
            row["source"],
            tokens.as_record(),
            console.as_record(),
            row["prelude"],
        ]
        supplied["final_directories"] = {
            name: _directory_inventory(workspace / name, _read, events)
            for name in _ORDINARY_DIRECTORIES
        }
        inspection = inspect_supplied_attempt(
            workspace,
            row,
            run,
            binding,
            prepared_run,
            active_reader() or physical_read_bytes,
        )
        events.extend(inspection.events)
        _require(
            inspection.unit is not None and not inspection.events,
            "ordinary captured source did not qualify",
        )
        row.update(generation="captured-unverified", mode=inspection.authenticated_mode)
    except Exception as error:
        retain(events, error, phase="ordinary-member", path=workspace)
        row.update(generation="failed", failure=f"{type(error).__name__}: {error}")
        supplied["final_directories"] = {}
        for name in _ORDINARY_DIRECTORIES:
            try:
                if (workspace / name).is_dir():
                    supplied["final_directories"][name] = _directory_inventory(
                        workspace / name, _read, events
                    )
            except Exception as inventory_error:
                retain(
                    events,
                    inventory_error,
                    phase="ordinary-failure-census",
                    path=workspace / name,
                )
    row["secondary_failures"] = facts(events)
    write_bytes(
        workspace / "result.json",
        encode_json(row, path=workspace, events=events),
        events=events,
    )
    run_events.extend(events)
    if isinstance(primary(events), ProcessCleanupError):
        raise primary(events)
