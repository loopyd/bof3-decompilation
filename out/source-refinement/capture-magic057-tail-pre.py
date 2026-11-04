"""Capture MAGIC057 public 32-byte PRE images without changing its 35-byte layout."""

from dataclasses import asdict
import hashlib
import json
from pathlib import Path

from harness.build.compiler import (
    DEFAULT_COMPILER_ID,
    build_compiler_arguments,
    load_compiler_configuration,
    resolve_compiler_settings,
)
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.process import run_bounded
from harness.domain.functions import parse_function_records
from harness.domain.layout import parse_splat_layout
from harness.domain.tags import parse_progress_tags
from harness.match.extraction import read_function_bytes
from harness.match.members import capture_members
from harness.match.validation import _exact_evidence

ROOT = Path(__file__).resolve().parents[2]
PLAN = "out/source-refinement/magic057-tail-boundary-draft/report.json"
PLAN_SHA = "0e80abf1002451654852420a06bc0f869861096cf190d0859c403327309a79f2"
OUT = ROOT / "out/source-refinement/magic057-tail-boundary-public-pre"
TARGET = "emi/bmagic/magic057/03"
BINARY = "out/binaries/emi/bmagic/magic057/03.bin"
SPLAT = f"config/targets/{TARGET}/splat.yaml"
LIMIT = 64 * 1024 * 1024
FUNCTION_SHA = "b0f3eb35b529fbeb3a0af17e770ebe867315b41a83b504d1adaf066c0eedbc68"


def sha(content):
    return hashlib.sha256(content).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def read(batch, name, *, absent=False):
    return batch.read(
        ROOT / name,
        max_bytes=1 if absent else LIMIT,
        include_metadata=True,
        single_link=True,
    )


def main():
    require(Path.cwd() == ROOT, "run from the repository root")
    plan_bytes = (ROOT / PLAN).read_bytes()
    require(sha(plan_bytes) == PLAN_SHA, "review and repin the changed MAGIC057 draft")
    plan = json.loads(plan_bytes)
    require(
        plan["target"] == TARGET
        and len(plan["inputs"]) == 143
        and sum(value is not None for value in plan["inputs"].values()) == 75,
        "reviewed target/input closure changed",
    )
    sources = [
        row for row in plan["profiles"]["all_target_sources"] if row["strict_records"]
    ]
    require(
        len(sources) == 2
        and {row["strict_records"][0]["address"] for row in sources}
        == {"0x801EFABC", "0x801EF81C"},
        "reviewed two-member pool changed",
    )
    OUT.mkdir(exist_ok=False)
    (OUT / "plan.json").write_bytes(plan_bytes)
    summary = {
        "status": "running",
        "scope": "MAGIC057 two public 32-byte PRE images only, before the terminal layout repair.",
        "plan_sha256": PLAN_SHA,
        "members": [],
        "limit": "Public native matches do not admit the current 35-byte C boundary or credit its three-byte suffix. Layout repair, POST, payload and independent acceptance remain pending.",
    }
    save(OUT / "summary.json", summary)
    try:
        with acquire_writer(ROOT):
            with InputBatch(ROOT) as inputs:
                states = {}
                images = {}
                pins = {
                    **plan["inputs"],
                    **plan["evidence_files"],
                    PLAN: {"sha256": PLAN_SHA, "size": len(plan_bytes)},
                }
                for name, expected in pins.items():
                    state, content = read(inputs, name, absent=expected is None)
                    require(
                        (state is None and content is None)
                        if expected is None
                        else (
                            state is not None
                            and state["sha256"] == expected["sha256"]
                            and len(content) == expected["size"]
                        ),
                        f"reviewed input changed: {name}",
                    )
                    states[name] = state
                    if name in {BINARY, SPLAT, *[row["source"] for row in sources]}:
                        images[name] = content
                index_state, index = read(inputs, ".git/index")
                require(index_state is not None, "raw index is absent")
                states[".git/index"] = index_state
                (OUT / "index").write_bytes(index)
                save(OUT / "input-states.json", states)
                owned = OUT / "original" / SPLAT
                owned.parent.mkdir(parents=True, exist_ok=True)
                owned.write_bytes(images[SPLAT])
                save(
                    OUT / "owned-pre.json",
                    {
                        "present": {SPLAT: states[SPLAT]},
                        "absent": [plan["tail"]["future_asset"]],
                        "index": index_state,
                    },
                )
                binary = images[BINARY]
                require(
                    len(binary) == 3807
                    and binary
                    == (ROOT / plan["shipped_identity"]["payload_path"]).read_bytes(),
                    "normalized/shipped payload differs",
                )
                full = binary[3772:3807]
                function = binary[3772:3804]
                tail = binary[3804:3807]
                require(
                    len(full) == 35
                    and len(function) == 32
                    and len(tail) == 3
                    and full == function + tail
                    and sha(function) == FUNCTION_SHA
                    and tail.hex() == "0a0e0e"
                    and sha(full)
                    == plan["evidence_files"][
                        "out/source-refinement/magic057-tail-boundary-draft/original-full35.bin"
                    ]["sha256"]
                    and sha(tail) == plan["tail"]["future_expected_sha256"],
                    "reviewed complete35/function32/tail3 bytes changed",
                )
                for name, content in (
                    ("original-full35.bin", full),
                    ("original-function32.bin", function),
                    ("original-tail3.bin", tail),
                ):
                    (OUT / name).write_bytes(content)
                layout = parse_splat_layout(
                    ROOT / SPLAT, plan["shipped_identity"]["load_address"]
                )
                boundary = layout.find_containing_boundary(0x801EFABC)
                require(
                    boundary is not None
                    and asdict(boundary) == plan["original_boundary"]
                    and boundary.file_size == 35,
                    "current terminal C layout changed before PRE",
                )
                terminal_source = plan["profiles"]["selected_source"]
                try:
                    capture_members(ROOT, terminal_source)
                except ValueError as error:
                    require(
                        str(error)
                        == "comparison member has an invalid original byte range",
                        "unexpected strict admission refusal",
                    )
                    refusal = {
                        "source": terminal_source,
                        "selector": plan["selector"],
                        "boundary": asdict(boundary),
                        "error": str(error),
                        "layout_admitted": False,
                        "limit": "Expected structural refusal of the unchanged35B C boundary, separate from public32B byte proof.",
                    }
                else:
                    raise ValueError(
                        "strict capture unexpectedly admitted the terminal35B C boundary"
                    )
                save(OUT / "strict-layout-refusal.json", refusal)

                flags, compilers = load_compiler_configuration(ROOT)
                prepared = []
                for row in sources:
                    source = row["source"]
                    text = images[source].decode()
                    records = parse_function_records(text)
                    expected = row["strict_records"][0]
                    require(len(records) == 1, f"not an ordinary singleton: {source}")
                    record = records[0]
                    require(
                        record.kind == expected["kind"] == "function"
                        and record.address == int(expected["address"], 16)
                        and record.spelling == expected["name"]
                        and list(parse_progress_tags(record.metadata))
                        == ["exact", 100.0, "none"],
                        f"metadata identity/progress changed: {source}",
                    )
                    settings = resolve_compiler_settings(source, text, flags, compilers)
                    arguments = build_compiler_arguments(ROOT, settings.flags)
                    require(
                        (settings.compiler_id or DEFAULT_COMPILER_ID)
                        == row["compiler"]
                        == plan["profiles"]["compiler"]
                        and settings.flags is row["flags"] is None
                        and arguments[1:] == plan["profiles"]["ordered_arguments"],
                        f"effective profile changed: {source}",
                    )
                    start = record.address - plan["shipped_identity"]["load_address"]
                    original = binary[start : start + 32]
                    require(
                        len(original) == 32 and sha(original) == FUNCTION_SHA,
                        f"reviewed32B original changed: {source}",
                    )
                    if record.address == 0x801EF81C:
                        admitted = capture_members(ROOT, source)
                        require(
                            len(admitted["functions"]) == 1
                            and not admitted["data_sections"]
                            and admitted["functions"][0]["original"] == original
                            and admitted["functions"][0]["file_start"] == start,
                            "other32B member strict admission changed",
                        )
                    metadata = {
                        "selector": f"{TARGET}@0x{record.address:08X}",
                        "source": source,
                        "symbol": record.spelling,
                        "source_sha256": sha(images[source]),
                        "metadata": record.metadata,
                        "metadata_sha256": sha(record.metadata.encode()),
                        "implementation": text[
                            record.implementation_start : record.implementation_end
                        ],
                        "compiler": settings.compiler_id or DEFAULT_COMPILER_ID,
                        "flags": settings.flags,
                        "ordered_arguments": arguments,
                        "file_start": start,
                        "address": record.address,
                        "original_sha256": sha(original),
                    }
                    prepared.append((metadata, original))

                for metadata, original in prepared:
                    member_dir = OUT / f"{metadata['address']:08X}"
                    member_dir.mkdir()
                    (member_dir / "source.c").write_bytes(images[metadata["source"]])
                    save(member_dir / "metadata-profile.json", metadata)
                    for operation in ("asm-diff", "byte-match"):
                        destination = member_dir / operation
                        destination.mkdir()
                        command = [
                            "bin/harness",
                            "lift",
                            operation,
                            metadata["selector"],
                            "--json",
                            "--image-evidence",
                        ]
                        if operation == "asm-diff":
                            command += ["--detail", "full"]
                        with (
                            (destination / "stdout").open("wb") as stdout,
                            (destination / "stderr").open("wb") as stderr,
                        ):

                            def retain(stream, chunk):
                                (stdout if stream == "stdout" else stderr).write(chunk)

                            result = run_bounded(
                                ROOT, command, output_limit=LIMIT, on_output=retain
                            )
                        save(destination / "process.json", result)
                        require(
                            not result["failure"] and result["exit_code"] == 0,
                            f"public PRE failed: {metadata['selector']} {operation}",
                        )
                        check = {
                            "argv": command,
                            "target": TARGET,
                            "selector": metadata["selector"],
                            "source": metadata["source"],
                            "function": metadata["symbol"],
                        }
                        require(
                            _exact_evidence(check, result["stdout"], ROOT),
                            "public exact evidence invalid",
                        )
                        report = json.loads(result["stdout"])
                        save(destination / "comparison.json", report)
                        require(
                            report["original_size"] == report["current_size"] == 32,
                            "public PRE did not cover precisely32B",
                        )
                        linked = Path(report["current_object"]).with_suffix(".linked.o")
                        require(
                            linked.is_absolute() and linked.is_relative_to(ROOT),
                            "actual linked output escaped repository",
                        )
                        # Copy before the next command can overwrite this function's ELF.
                        with InputBatch(ROOT) as artifacts:
                            linked_state, elf = read(
                                artifacts, linked.relative_to(ROOT).as_posix()
                            )
                            require(
                                linked_state is not None, "actual linked ELF is absent"
                            )
                            current = read_function_bytes(
                                elf,
                                function_name=metadata["symbol"],
                                address=metadata["address"],
                            ).content
                            require(
                                current == original
                                and len(current) == 32
                                and sha(current) == FUNCTION_SHA
                                and report["function_image"]
                                == {
                                    "original_sha256": sha(original),
                                    "current_sha256": sha(current),
                                },
                                "actual complete32B ELF image differs",
                            )
                            (destination / "current.linked.o").write_bytes(elf)
                            save(destination / "linked-state.json", linked_state)
                        (destination / "original.bin").write_bytes(original)
                        (destination / "current.bin").write_bytes(current)
                        summary["members"].append(
                            {
                                "selector": metadata["selector"],
                                "operation": operation,
                                "size": 32,
                                "original_sha256": sha(original),
                                "current_sha256": sha(current),
                                "saved_elf": str(
                                    (destination / "current.linked.o").relative_to(ROOT)
                                ),
                                "elf_sha256": sha(elf),
                            }
                        )
                        save(OUT / "summary.json", summary)
                        print(
                            f"{metadata['selector']} {operation}: complete32B exact; actual ELF saved",
                            flush=True,
                        )
                for name, expected in states.items():
                    observed, _ = read(inputs, name, absent=expected is None)
                    require(observed == expected, f"terminal input/index drift: {name}")
        require(len(summary["members"]) == 4, "incomplete two-member/two-command PRE")
        summary.update(
            status="complete",
            selected_members=2,
            public_comparisons=4,
            original_function_bytes=64,
            terminal_original_span=35,
            retained_suffix_bytes=3,
            strict_terminal_layout_admitted=False,
            guards_and_writer_closed=True,
        )
        save(OUT / "summary.json", summary)
    except BaseException as error:
        summary.update(status="failed", error=f"{type(error).__name__}: {error}")
        save(OUT / "summary.json", summary)
        raise


if __name__ == "__main__":
    main()
