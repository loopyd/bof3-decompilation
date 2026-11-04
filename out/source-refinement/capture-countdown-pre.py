"""Capture complete PRE images for the six approved countdown functions only."""

from __future__ import annotations

import argparse
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
from harness.common.inputs import InputBatch, relative
from harness.common.lease import acquire_writer
from harness.common.process import ProcessCleanupError
from harness.domain.functions import parse_function_records
from harness.match.extraction import read_function_bytes
from harness.match.images import capture_source_images
from harness.match.members import capture_members

ROOT = Path(__file__).resolve().parents[2]
PLAN = "out/source-refinement/countdown-ready-draft/plan.json"
PLAN_SHA = "2b492891e7d9d6e72a47d666e563ea16c60011986d5bbcf1bce48e83fdf84c8e"
VARIANTS = "config/compiler/variants.json"
VARIANTS_SHA = "96685df945605d42971a188afccbb19dbfc0cbe0faffdb981e5fc9051292e9fd"
OUT = ROOT / "out/source-refinement/countdown-pre"
LIMIT = 64 * 1024 * 1024


def require(condition, message):
    if not condition:
        raise ValueError(message)


def sha(content):
    return hashlib.sha256(content).hexdigest()


def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def read(inputs, name):
    return inputs.read(
        ROOT / relative(name),
        max_bytes=LIMIT,
        include_metadata=True,
        single_link=True,
    )


def main():
    argparse.ArgumentParser(description=__doc__).parse_args()
    require(Path.cwd() == ROOT, "run from the repository root")
    require(not OUT.exists(), "countdown PRE output must be new")
    plan_bytes = (ROOT / PLAN).read_bytes()
    require(sha(plan_bytes) == PLAN_SHA, "approved countdown plan changed")
    plan = json.loads(plan_bytes)
    members = sorted(plan["members"], key=lambda m: m["selector"])
    require(
        len(members) == 6
        and len({m["selector"] for m in members}) == 6
        and len({m["source"] for m in members}) == 6
        and {m["target"] for m in members}
        == {"emi/world03/area119/13", "emi/world03/area134/13"},
        "approved six-source and two-owner scope changed",
    )
    require(
        len(plan["input_states"]) == 168
        and sum(v is not None for v in plan["input_states"].values()) == 86
        and len(plan["new_paths_absent"]) == 3,
        "approved PRE input closure changed",
    )
    owned = set(plan["owned_paths"]["final"]) - set(plan["new_paths_absent"])
    require(len(owned) == 11, "expected eleven existing owned inputs")
    OUT.mkdir(exist_ok=False)
    (OUT / "plan.json").write_bytes(plan_bytes)
    summary = {
        "status": "running",
        "stage": "PRE only",
        "task": "3.21",
        "plan_sha256": PLAN_SHA,
        "supplemental_configuration_sha256": {VARIANTS: VARIANTS_SHA},
        "members": [],
        "limit": "Fresh complete selected images only. No extraction, source edit, metadata promotion, payload claim or semantic acceptance.",
    }
    save(OUT / "summary.json", summary)
    try:
        with acquire_writer(ROOT):
            with InputBatch(ROOT) as inputs:
                states = {}
                for name, expected in plan["input_states"].items():
                    state, content = read(inputs, name)
                    require(state == expected, f"approved PRE input changed: {name}")
                    states[name] = state
                    if name in owned:
                        destination = OUT / "original" / name
                        destination.parent.mkdir(parents=True, exist_ok=True)
                        destination.write_bytes(content)
                for name, digest in ((PLAN, PLAN_SHA), (VARIANTS, VARIANTS_SHA)):
                    state, _ = read(inputs, name)
                    require(
                        state and state["sha256"] == digest,
                        f"reviewed input changed: {name}",
                    )
                    states[name] = state
                index_state, index = read(inputs, ".git/index")
                require(index_state is not None, "raw index is absent")
                states[".git/index"] = index_state
                (OUT / "index").write_bytes(index)
                save(
                    OUT / "owned-pre.json",
                    {
                        "present": {name: states[name] for name in sorted(owned)},
                        "absent": plan["new_paths_absent"],
                        "index": index_state,
                    },
                )

                def verify_inputs():
                    for name, expected in states.items():
                        observed, _ = read(inputs, name)
                        require(observed == expected, f"input or index drift: {name}")

                def bind_report_inputs(report):
                    for name, expected in {
                        **report["native_inputs"],
                        **report["include_inputs"],
                    }.items():
                        observed, _ = read(inputs, name)
                        require(
                            observed == expected,
                            f"captured native input changed: {name}",
                        )
                        if name in states:
                            require(
                                states[name] == observed,
                                f"native input differs from PRE: {name}",
                            )
                        states[name] = observed

                # Strictly admit all six ordinary sources before the first compiler.
                prepared = []
                flags, compilers = load_compiler_configuration(ROOT)
                for member in members:
                    source = member["source"]
                    _, content = read(inputs, source)
                    require(
                        sha(content) == member["source_sha256"],
                        f"source changed: {source}",
                    )
                    text = content.decode("utf-8")
                    records = parse_function_records(text)
                    require(
                        len(records) == 1, f"PRE source is not a singleton: {source}"
                    )
                    record = records[0]
                    reviewed = member["native_range"]
                    require(
                        record.address == int(reviewed["address_start"], 16)
                        and record.kind == "function"
                        and record.spelling == member["symbol"]
                        and record.metadata == member["metadata_original"]
                        and text[
                            record.implementation_start : record.implementation_end
                        ]
                        == member["implementation"],
                        f"PRE identity, metadata, body or ABI changed: {source}",
                    )
                    settings = resolve_compiler_settings(source, text, flags, compilers)
                    profile = asdict(settings)
                    arguments = build_compiler_arguments(ROOT, settings.flags)
                    require(
                        profile == member["current_profile"]
                        and arguments == member["ordered_arguments"]
                        and (settings.compiler_id or DEFAULT_COMPILER_ID)
                        == "gcc-2.7.2-psx",
                        f"effective PRE profile changed: {source}",
                    )
                    captured = capture_members(ROOT, source)
                    require(
                        captured["target"] == member["target"]
                        and len(captured["functions"]) == 1
                        and not captured["data_sections"]
                        and not any(captured["section_placements"].values()),
                        f"PRE target, member or placed-data closure changed: {source}",
                    )
                    function = captured["functions"][0]
                    require(
                        function["selector"] == member["selector"]
                        and function["symbol"] == member["symbol"]
                        and function["file_start"] == reviewed["file_start"]
                        and len(function["original"]) == reviewed["size"] == 56
                        and sha(function["original"]) == reviewed["original_sha256"],
                        f"complete PRE original differs: {source}",
                    )
                    for name, expected in captured["inputs"].items():
                        require(
                            name in states
                            and states[name] is not None
                            and states[name]["sha256"] == expected["sha256"],
                            f"member input is outside approved PRE: {name}",
                        )
                    prepared.append((member, function, profile, arguments))
                require(len(prepared) == 6, "incomplete PRE admission")
                verify_inputs()
                save(OUT / "input-states.json", states)

                for member, function, profile, arguments in prepared:
                    verify_inputs()
                    source = member["source"]
                    area = member["target"].split("/")[-2]
                    destination = OUT / f"{area}13-{function['address']:08X}"
                    destination.mkdir()
                    original = function["original"]
                    (destination / "original.bin").write_bytes(original)
                    save(
                        destination / "metadata-profile.json",
                        {
                            "selector": member["selector"],
                            "source": source,
                            "symbol": function["symbol"],
                            "metadata": member["metadata_original"],
                            "metadata_sha256": sha(
                                member["metadata_original"].encode()
                            ),
                            "profile": profile,
                            "ordered_arguments": arguments,
                            "reviewed_original": member["native_range"],
                        },
                    )
                    comparison = capture_source_images(ROOT, source, destination)
                    report = comparison.report
                    save(destination / "comparison.json", report)
                    linked = Path(report["current_object"]).with_suffix(".linked.o")
                    require(
                        linked.is_absolute() and linked.is_relative_to(ROOT),
                        "linked ELF escaped the repository",
                    )
                    linked_name = linked.relative_to(ROOT).as_posix()
                    require(
                        linked_name in report["native_outputs"],
                        "observed native outputs omit linked ELF",
                    )
                    elf = None
                    artifacts = {}
                    # Retain actual complete outputs before a mismatch can stop PRE.
                    with InputBatch(ROOT) as native:
                        for name, expected in report["native_outputs"].items():
                            observed, image = read(native, name)
                            require(
                                observed == expected,
                                f"native artifact changed after capture: {name}",
                            )
                            saved = destination / "native" / name
                            saved.parent.mkdir(parents=True, exist_ok=True)
                            saved.write_bytes(image)
                            require(
                                sha(saved.read_bytes()) == observed["sha256"],
                                "saved native artifact differs",
                            )
                            artifacts[name] = {
                                "state": observed,
                                "saved_at": saved.relative_to(ROOT).as_posix(),
                            }
                            if name == linked_name:
                                elf = image
                    require(elf is not None, "linked ELF capture is missing")
                    (destination / "current.linked.o").write_bytes(elf)
                    save(destination / "native-artifacts.json", artifacts)
                    current = read_function_bytes(
                        elf,
                        function_name=function["symbol"],
                        address=function["address"],
                    ).content
                    (destination / "current.bin").write_bytes(current)
                    emitted_rows = []
                    for ordinal, emitted in enumerate(comparison.ranges):
                        name = f"emitted-{ordinal}.bin"
                        (destination / name).write_bytes(emitted.content)
                        emitted_rows.append(
                            {
                                "source": emitted.source,
                                "address": emitted.address,
                                "original_file_start": emitted.original_file_start,
                                "symbol": emitted.symbol,
                                "section": emitted.section,
                                "owners": list(emitted.owners),
                                "size": len(emitted.content),
                                "sha256": sha(emitted.content),
                                "saved_at": name,
                            }
                        )
                    save(destination / "emitted-ranges.json", emitted_rows)
                    bind_report_inputs(report)
                    require(
                        report["schema"] == "harness.byte-match-one/v1"
                        and report.get("fresh_compilation") is True
                        and report["status"] == "exact_match"
                        and report["byte_match"] is True
                        and report["exact_match"] is True
                        and report["original_size"] == report["current_size"] == 56
                        and report["function"] == function["symbol"]
                        and int(report["address"], 16) == function["address"]
                        and report["source_image"]
                        == {
                            "source": source,
                            "kind": "current",
                            "sha256": member["source_sha256"],
                            "size": states[source]["metadata"]["st_size"],
                        }
                        and report["function_image"]
                        == {
                            "original_sha256": sha(original),
                            "current_sha256": sha(current),
                        }
                        and current == original
                        and len(current) == 56,
                        f"fresh complete 56B PRE comparison differs: {member['selector']}",
                    )
                    require(
                        len(comparison.ranges) == 1,
                        "PRE comparison emitted an unexpected range count",
                    )
                    emitted = comparison.ranges[0]
                    require(
                        emitted.source == source
                        and emitted.address == function["address"]
                        and emitted.original_file_start == function["file_start"]
                        and emitted.symbol == function["symbol"]
                        and emitted.owners == (member["selector"],)
                        and emitted.section is None
                        and emitted.content == current == original,
                        f"complete emitted PRE range differs: {member['selector']}",
                    )
                    verify_inputs()
                    summary["members"].append(
                        {
                            "selector": member["selector"],
                            "source": source,
                            "size": len(current),
                            "original_sha256": sha(original),
                            "current_sha256": sha(current),
                            "saved_elf": (destination / "current.linked.o")
                            .relative_to(ROOT)
                            .as_posix(),
                            "elf_sha256": sha(elf),
                            "comparison": (destination / "comparison.json")
                            .relative_to(ROOT)
                            .as_posix(),
                        }
                    )
                    save(OUT / "summary.json", summary)
                    save(OUT / "input-states.json", states)
                    print(
                        f"PRE {member['selector']}: complete 56B exact image retained",
                        flush=True,
                    )
                verify_inputs()
        require(len(summary["members"]) == 6, "incomplete six-member PRE capture")
        summary.update(
            status="complete PRE captures",
            member_count=6,
            complete_original_bytes=336,
            guards_and_writer_closed=True,
        )
        save(OUT / "summary.json", summary)
    except BaseException as error:
        summary.update(
            status="failed", error_type=type(error).__name__, error=str(error)
        )
        if isinstance(error, ProcessCleanupError):
            summary["cleanup"] = "unconfirmed; stop further native work and recovery"
            if error.observed is not None:
                save(OUT / "cleanup-failure.json", error.observed)
        save(OUT / "summary.json", summary)
        raise


if __name__ == "__main__":
    main()
