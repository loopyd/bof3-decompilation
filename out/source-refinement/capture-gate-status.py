"""Capture only the reviewed twelve gate/status members at PRE/private/shared."""

import argparse
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
from harness.domain.functions import parse_function_records
from harness.match.extraction import read_function_bytes
from harness.match.images import capture_source_images
from harness.match.members import capture_members

ROOT = Path(__file__).resolve().parents[2]
PLAN = ROOT / "out/source-refinement/gate-status-ready-draft/plan.json"
PLAN_SHA = "75c6b8e2a788ee8909dd1d09556363c2a6dcc05f04901b6471397862d32644d0"
LIMIT = 64 * 1024 * 1024


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
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("stage", choices=("pre", "private", "shared"))
    stage = parser.parse_args().stage
    require(Path.cwd() == ROOT, "run from the repository root")
    plan_bytes = PLAN.read_bytes()
    require(
        sha(plan_bytes) == PLAN_SHA, "review and repin the changed gate/status plan"
    )
    plan = json.loads(plan_bytes)
    members = {member["selector"]: member for member in plan["members"]}
    require(len(members) == 12 and len(plan["batches"]) == 3, "reviewed pool changed")
    pins = dict(plan["current_input_sha256"])
    require(len(pins) == 88 and len(plan["absent_paths"]) == 10, "PRE pins changed")
    removed = []
    if stage != "pre":
        removed = plan[
            "private_removed_sources" if stage == "private" else "removed_sources"
        ]
        for name in removed:
            del pins[name]
        pins.update(
            plan[
                "private_output_sha256"
                if stage == "private"
                else "proposed_output_sha256"
            ]
        )
    absent = (set(plan["absent_paths"]) - pins.keys()) | set(removed)
    require(
        (len(pins), len(absent))
        == {"pre": (88, 10), "private": (79, 19), "shared": (80, 18)}[stage],
        "stage pins changed",
    )
    owned = {
        name
        for batch in plan["batches"]
        for name in [*batch["sources_removed"], batch["manifest"], batch["splat"]]
    } | {"tools/python/harness/macros/index.py"}
    require(
        len(owned) == 19 and owned <= plan["current_input_sha256"].keys(),
        "owned PRE set changed",
    )
    output = ROOT / f"out/source-refinement/gate-status-{stage}"
    output.mkdir(exist_ok=False)
    (output / "plan.json").write_bytes(plan_bytes)
    summary = {
        "stage": stage,
        "status": "running",
        "plan_sha256": PLAN_SHA,
        "sources": [],
        "limit": "Complete selected native images; independent review and remaining gates are parent-owned.",
        "unchanged_work_state": "Nine sources/eighteen members remain pinned; none is compiled by this helper.",
    }
    save(output / "summary.json", summary)
    try:
        with acquire_writer(ROOT):
            with InputBatch(ROOT) as inputs:
                states = {}
                for name, digest in sorted(pins.items()):
                    state, content = read(inputs, name)
                    require(
                        state is not None and state["sha256"] == digest,
                        f"input changed: {name}",
                    )
                    states[name] = state
                    if stage == "pre" and name in owned:
                        destination = output / "original" / name
                        destination.parent.mkdir(parents=True, exist_ok=True)
                        destination.write_bytes(content)
                for name in sorted(absent):
                    state, _ = read(inputs, name, absent=True)
                    require(state is None, f"expected absent input: {name}")
                if stage == "pre":
                    index_state, index = read(inputs, ".git/index")
                    require(index_state is not None, "raw index is absent")
                    (output / "index").write_bytes(index)
                    states[".git/index"] = index_state
                    save(
                        output / "owned-pre.json",
                        {
                            "present": {name: states[name] for name in sorted(owned)},
                            "absent": sorted(absent),
                            "index": index_state,
                        },
                    )
                save(
                    output / "input-states.json",
                    {"present": states, "absent": sorted(absent)},
                )

                # Admit every selected source before starting the first compiler.
                prepared = []
                flags, compilers = load_compiler_configuration(ROOT)
                for batch in plan["batches"]:
                    if stage == "private" and not batch["private_first"]:
                        continue
                    selected = {key: members[key] for key in batch["members"]}
                    sources = (
                        batch["sources_removed"]
                        if stage == "pre"
                        else [batch["destination"]]
                    )
                    for source in sources:
                        text = (ROOT / source).read_text()
                        records = parse_function_records(text)
                        expected = {
                            int(member["native_range"]["address_start"], 16): member
                            for member in selected.values()
                            if stage != "pre" or member["source"] == source
                        }
                        require(
                            len(records) == len(expected)
                            and {record.address for record in records}
                            == expected.keys(),
                            f"record closure changed: {source}",
                        )
                        settings = resolve_compiler_settings(
                            source, text, flags, compilers
                        )
                        profile = {
                            "compiler_id": settings.compiler_id,
                            "flags": list(settings.flags)
                            if settings.flags is not None
                            else None,
                        }
                        arguments = build_compiler_arguments(ROOT, settings.flags)
                        for record in records:
                            member = expected[record.address]
                            implementation = text[
                                record.implementation_start : record.implementation_end
                            ]
                            require(
                                record.metadata == member["metadata"]
                                and sha(record.metadata.encode())
                                == member["metadata_sha256"],
                                f"metadata/progress changed: {member['selector']}",
                            )
                            require(
                                (
                                    record.kind == "function"
                                    and record.spelling == member["symbol"]
                                    and sha(implementation.encode())
                                    == member["implementation_sha256"]
                                )
                                if stage == "pre"
                                else (
                                    record.kind == "template"
                                    and record.spelling == "WORLD_DEFINE_GATE_STATUS"
                                    and implementation == member["draft_invocation"]
                                ),
                                f"implementation/invocation changed: {member['selector']}",
                            )
                            require(
                                profile == member["profile_settings"]
                                and arguments == member["ordered_arguments"]
                                and (settings.compiler_id or DEFAULT_COMPILER_ID)
                                == "gcc-2.7.2-psx",
                                f"effective profile changed: {source}",
                            )
                        captured = capture_members(ROOT, source)
                        require(
                            captured["target"] == batch["target"]
                            and captured["binary"] == batch["binary"]
                            and not captured["data_sections"],
                            f"ownership/data changed: {source}",
                        )
                        require(
                            len(captured["functions"]) == len(expected)
                            and {
                                function["address"]
                                for function in captured["functions"]
                            }
                            == expected.keys(),
                            f"native member closure changed: {source}",
                        )
                        for function in captured["functions"]:
                            member = expected[function["address"]]
                            original = member["native_range"]
                            require(
                                function["selector"] == member["selector"]
                                and function["symbol"] == member["symbol"]
                                and function["file_start"] == original["file_start"]
                                and len(function["original"]) == original["size"] == 40
                                and sha(function["original"])
                                == original["original_sha256"],
                                f"original range changed: {member['selector']}",
                            )
                        prepared.append(
                            (source, captured, expected, profile, arguments)
                        )
                require(
                    sum(len(item[2]) for item in prepared)
                    == (11 if stage == "private" else 12),
                    "incomplete stage admission",
                )
                require(
                    len(prepared) == {"pre": 12, "private": 2, "shared": 3}[stage],
                    "unexpected source count",
                )

                for source, captured, expected, profile, arguments in prepared:
                    destination = output / source.replace("/", "_")
                    destination.mkdir()
                    comparison = capture_source_images(ROOT, source, destination)
                    report = comparison.report
                    save(destination / "comparison.json", report)
                    require(
                        report.get("fresh_compilation") is True,
                        f"fresh compilation missing: {source}",
                    )
                    if report["schema"] == "harness.byte-match-one/v1":
                        require(
                            report["byte_match"] and report["exact_match"],
                            f"ordinary comparison differs: {source}",
                        )
                        linked = Path(report["current_object"]).with_suffix(".linked.o")
                        native_outputs = report["native_outputs"]
                    else:
                        require(
                            report["schema"] == "bof3.grouped-comparison/v8"
                            and report["all_native_bytes_match"]
                            and not report["data_sections"],
                            f"grouped comparison differs: {source}",
                        )
                        linked = Path(report["placement"]["path"])
                        native_outputs = report["outputs"]
                    require(
                        linked.is_absolute() and linked.is_relative_to(ROOT),
                        "linked output escaped repository",
                    )
                    linked_name = linked.relative_to(ROOT).as_posix()
                    require(
                        linked_name in native_outputs,
                        "linked ELF omitted from observed outputs",
                    )
                    # Close this observation before the next source can rewrite native artifacts.
                    with InputBatch(ROOT) as artifacts:
                        linked_state, elf = read(artifacts, linked_name)
                        require(
                            linked_state == native_outputs[linked_name],
                            f"actual ELF changed after capture: {source}",
                        )
                        (destination / "current.linked.o").write_bytes(elf)
                        save(destination / "linked-state.json", linked_state)
                    require(
                        len(comparison.ranges) == len(expected),
                        f"unexpected emitted ranges: {source}",
                    )
                    rows = []
                    for function in captured["functions"]:
                        member = expected[function["address"]]
                        emitted = [
                            item
                            for item in comparison.ranges
                            if item.address == function["address"]
                            and item.symbol == function["symbol"]
                        ]
                        require(
                            len(emitted) == 1,
                            f"emitted identity ambiguous: {member['selector']}",
                        )
                        emitted = emitted[0]
                        current = read_function_bytes(
                            elf,
                            function_name=function["symbol"],
                            address=function["address"],
                        ).content
                        original = function["original"]
                        require(
                            emitted.section is None
                            and emitted.original_file_start == function["file_start"]
                            and emitted.content == current == original
                            and len(current) == 40
                            and sha(current)
                            == member["native_range"]["original_sha256"],
                            f"complete actual ELF image differs: {member['selector']}",
                        )
                        prefix = f"{function['address']:08X}"
                        (destination / f"{prefix}-original.bin").write_bytes(original)
                        (destination / f"{prefix}-current.bin").write_bytes(current)
                        row = {
                            "selector": member["selector"],
                            "symbol": function["symbol"],
                            "source": source,
                            "size": 40,
                            "file_start": function["file_start"],
                            "original_sha256": sha(original),
                            "current_sha256": sha(current),
                            "metadata": member["metadata"],
                            "metadata_sha256": member["metadata_sha256"],
                            "profile": profile,
                            "ordered_arguments": arguments,
                        }
                        save(destination / f"{prefix}-metadata-profile.json", row)
                        rows.append(row)
                    summary["sources"].append(
                        {
                            "source": source,
                            "target": captured["target"],
                            "members": rows,
                            "comparison": str(
                                (destination / "comparison.json").relative_to(ROOT)
                            ),
                            "saved_elf": str(
                                (destination / "current.linked.o").relative_to(ROOT)
                            ),
                            "elf_sha256": sha(elf),
                        }
                    )
                    save(output / "summary.json", summary)
                    print(
                        f"{stage}: {source}: {len(rows)} complete exact 40-byte members",
                        flush=True,
                    )

                for name, expected in states.items():
                    state, _ = read(inputs, name)
                    require(state == expected, f"terminal input/index drift: {name}")
                for name in sorted(absent):
                    state, _ = read(inputs, name, absent=True)
                    require(state is None, f"terminal absent input appeared: {name}")
        summary.update(
            status="complete",
            member_count=sum(len(row["members"]) for row in summary["sources"]),
            complete_original_bytes=(440 if stage == "private" else 480),
            guards_and_writer_closed=True,
        )
        require(
            summary["member_count"] == (11 if stage == "private" else 12),
            "incomplete final member capture",
        )
        save(output / "summary.json", summary)
    except BaseException as error:
        summary.update(status="failed", error=f"{type(error).__name__}: {error}")
        save(output / "summary.json", summary)
        raise


if __name__ == "__main__":
    main()
