"""Capture the two complete countdown groups after private or shared application."""

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
PRE_SUMMARY = "out/source-refinement/countdown-pre/summary.json"
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
        ROOT / relative(name), max_bytes=LIMIT, include_metadata=True, single_link=True
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("stage", choices=("private", "shared"))
    parser.add_argument("--pre-summary-sha256", required=True)
    args = parser.parse_args()
    require(Path.cwd() == ROOT, "run from the repository root")
    require(
        len(args.pre_summary_sha256) == 64
        and all(c in "0123456789abcdef" for c in args.pre_summary_sha256),
        "supply the independently reviewed PRE summary SHA256",
    )
    plan_bytes = (ROOT / PLAN).read_bytes()
    require(sha(plan_bytes) == PLAN_SHA, "approved countdown plan changed")
    plan = json.loads(plan_bytes)
    members = {m["selector"]: m for m in plan["members"]}
    outputs = plan[
        "private_output_sha256" if args.stage == "private" else "shared_output_sha256"
    ]
    require(
        len(members) == 6
        and len(plan["batches"]) == 2
        and len(plan["source_deletions"]) == 6
        and len(plan["input_states"]) == 168,
        "approved countdown scope changed",
    )
    require(
        len(outputs) == (6 if args.stage == "private" else 8),
        "approved stage output count changed",
    )
    require(
        set(outputs) <= plan["input_states"].keys(),
        "stage outputs exceed approved inputs",
    )
    out = ROOT / f"out/source-refinement/countdown-{args.stage}"
    require(not out.exists(), "stage capture output must be new")
    out.mkdir(exist_ok=False)
    (out / "plan.json").write_bytes(plan_bytes)
    summary = {
        "status": "running",
        "stage": args.stage,
        "task": "3.21",
        "plan_sha256": PLAN_SHA,
        "pre_summary_sha256": args.pre_summary_sha256,
        "sources": [],
        "limit": "Complete current group captures and equality to reviewed PRE/original only. No source edits, metadata promotion, payload credit or semantic acceptance.",
    }
    save(out / "summary.json", summary)
    try:
        with acquire_writer(ROOT):
            with InputBatch(ROOT) as inputs:
                states = {}
                for name, expected in plan["input_states"].items():
                    observed, _ = read(inputs, name)
                    if name in plan["source_deletions"]:
                        require(observed is None, f"retired source remains: {name}")
                    elif name in outputs:
                        require(
                            observed and observed["sha256"] == outputs[name],
                            f"approved stage output differs: {name}",
                        )
                    else:
                        require(
                            observed == expected, f"preserved PRE input changed: {name}"
                        )
                    states[name] = observed
                for name, digest in (
                    (PLAN, PLAN_SHA),
                    (VARIANTS, VARIANTS_SHA),
                    (PRE_SUMMARY, args.pre_summary_sha256),
                ):
                    observed, content = read(inputs, name)
                    require(
                        observed and observed["sha256"] == digest,
                        f"reviewed input changed: {name}",
                    )
                    states[name] = observed
                    if name == PRE_SUMMARY:
                        pre = json.loads(content)
                require(
                    pre["plan_sha256"] == PLAN_SHA
                    and pre["status"] == "complete PRE captures"
                    and pre["member_count"] == 6
                    and pre["complete_original_bytes"] == 336
                    and pre["guards_and_writer_closed"] is True,
                    "PRE summary is incomplete or unclosed",
                )
                pre_rows = {m["selector"]: m for m in pre["members"]}
                require(
                    len(pre["members"]) == len(pre_rows) == 6
                    and pre_rows.keys() == members.keys(),
                    "reviewed PRE member closure differs",
                )
                pre_images = {}
                for selector, row in pre_rows.items():
                    member = members[selector]
                    require(
                        row["source"] == member["source"]
                        and row["size"] == 56
                        and row["original_sha256"]
                        == row["current_sha256"]
                        == member["native_range"]["original_sha256"],
                        f"reviewed PRE image facts differ: {selector}",
                    )
                    linked_name = relative(row["saved_elf"])
                    require(
                        linked_name.startswith("out/source-refinement/countdown-pre/"),
                        "saved PRE ELF escaped its capture directory",
                    )
                    parent = Path(linked_name).parent
                    images = {}
                    for label in ("original", "current"):
                        name = (parent / f"{label}.bin").as_posix()
                        observed, image = read(inputs, name)
                        require(
                            observed
                            and len(image) == 56
                            and sha(image) == row[f"{label}_sha256"],
                            f"saved complete PRE range changed: {name}",
                        )
                        states[name] = observed
                        images[label] = image
                    observed, elf = read(inputs, linked_name)
                    require(
                        observed and sha(elf) == row["elf_sha256"],
                        f"saved PRE ELF changed: {selector}",
                    )
                    states[linked_name] = observed
                    actual = read_function_bytes(
                        elf,
                        function_name=member["symbol"],
                        address=int(member["native_range"]["address_start"], 16),
                    ).content
                    require(
                        actual == images["current"] == images["original"]
                        and len(actual) == 56,
                        f"actual complete PRE ELF range differs: {selector}",
                    )
                    pre_images[selector] = actual
                index_state, index = read(inputs, ".git/index")
                require(index_state is not None, "raw index is absent")
                states[".git/index"] = index_state
                (out / "index").write_bytes(index)

                def verify_inputs():
                    for name, expected in states.items():
                        observed, _ = read(inputs, name)
                        require(
                            observed == expected,
                            f"input, PRE evidence or index drift: {name}",
                        )

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
                                f"native input differs from starting state: {name}",
                            )
                        states[name] = observed

                # Admit both whole groups before starting either native producer.
                prepared = []
                flags, compilers = load_compiler_configuration(ROOT)
                for batch in plan["batches"]:
                    source = batch["destination"]
                    selected = {s: members[s] for s in batch["members"]}
                    expected_count = (
                        2 if batch["target"] == "emi/world03/area119/13" else 4
                    )
                    require(
                        batch["target"]
                        in ("emi/world03/area119/13", "emi/world03/area134/13")
                        and len(selected) == expected_count
                        and {m["target"] for m in selected.values()}
                        == {batch["target"]},
                        "group ownership or reviewed 2/4 closure changed",
                    )
                    _, content = read(inputs, source)
                    text = content.decode("utf-8")
                    records = parse_function_records(text)
                    by_address = {
                        int(m["native_range"]["address_start"], 16): m
                        for m in selected.values()
                    }
                    require(
                        len(records) == expected_count
                        and {r.address for r in records} == by_address.keys(),
                        f"complete generated metadata closure differs: {source}",
                    )
                    settings = resolve_compiler_settings(source, text, flags, compilers)
                    profile = asdict(settings)
                    arguments = build_compiler_arguments(ROOT, settings.flags)
                    for record in records:
                        member = by_address[record.address]
                        invocation = f"WORLD_DEFINE_COUNTDOWN_TO_MODE({member['symbol']}, {member['parameters']['when_spent_mode']})"
                        require(
                            record.kind == "template"
                            and record.spelling == "WORLD_DEFINE_COUNTDOWN_TO_MODE"
                            and record.metadata == member["metadata_proposed"]
                            and text[
                                record.implementation_start : record.implementation_end
                            ]
                            == invocation,
                            f"approved invocation or metadata differs: {member['selector']}",
                        )
                        require(
                            profile == member["current_profile"]
                            and arguments == member["ordered_arguments"]
                            and (settings.compiler_id or DEFAULT_COMPILER_ID)
                            == "gcc-2.7.2-psx",
                            f"effective profile changed: {member['selector']}",
                        )
                    captured = capture_members(ROOT, source)
                    require(
                        captured["target"] == batch["target"]
                        and len(captured["functions"]) == expected_count
                        and {f["selector"] for f in captured["functions"]}
                        == selected.keys()
                        and not captured["data_sections"]
                        and not any(captured["section_placements"].values()),
                        f"grouped member or placed-data admission differs: {source}",
                    )
                    for function in captured["functions"]:
                        member = selected[function["selector"]]
                        reviewed = member["native_range"]
                        require(
                            function["symbol"] == member["symbol"]
                            and function["address"]
                            == int(reviewed["address_start"], 16)
                            and function["file_start"] == reviewed["file_start"]
                            and len(function["original"]) == reviewed["size"] == 56
                            and sha(function["original"]) == reviewed["original_sha256"]
                            and function["original"] == pre_images[member["selector"]],
                            f"complete original or PRE range differs: {member['selector']}",
                        )
                    for name, expected in captured["inputs"].items():
                        require(
                            name in states
                            and states[name] is not None
                            and states[name]["sha256"] == expected["sha256"],
                            f"member input is outside approved stage: {name}",
                        )
                    prepared.append((batch, captured, selected, profile, arguments))
                require(
                    len(prepared) == 2 and sum(len(p[2]) for p in prepared) == 6,
                    "incomplete whole-group admission",
                )
                verify_inputs()
                save(out / "input-states.json", states)

                for batch, captured, selected, profile, arguments in prepared:
                    verify_inputs()
                    source = batch["destination"]
                    destination = out / Path(source).stem
                    destination.mkdir()
                    for function in captured["functions"]:
                        (
                            destination / f"{function['address']:08X}-original.bin"
                        ).write_bytes(function["original"])
                    save(
                        destination / "metadata-profiles.json",
                        {
                            "target": batch["target"],
                            "source": source,
                            "profile": profile,
                            "ordered_arguments": arguments,
                            "members": [
                                {
                                    "selector": m["selector"],
                                    "symbol": m["symbol"],
                                    "metadata": m["metadata_proposed"],
                                    "original_range": m["native_range"],
                                }
                                for m in selected.values()
                            ],
                        },
                    )
                    comparison = capture_source_images(ROOT, source, destination)
                    report = comparison.report
                    save(destination / "comparison.json", report)
                    require(
                        report["schema"] == "bof3.grouped-comparison/v8",
                        "whole-group native report schema differs",
                    )
                    linked = Path(report["placement"]["path"])
                    require(
                        linked.is_absolute() and linked.is_relative_to(destination),
                        "linked group ELF escaped its stage output",
                    )
                    linked_name = linked.relative_to(ROOT).as_posix()
                    require(
                        linked_name in report["outputs"],
                        "observed group outputs omit the linked ELF",
                    )
                    elf = None
                    artifacts = {}
                    with InputBatch(ROOT) as native:
                        for name, expected in report["outputs"].items():
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
                    require(elf is not None, "complete group ELF capture is missing")
                    (destination / "current.linked.o").write_bytes(elf)
                    save(destination / "native-artifacts.json", artifacts)
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
                    rows, failures = [], []
                    for function in captured["functions"]:
                        member = selected[function["selector"]]
                        current = read_function_bytes(
                            elf,
                            function_name=function["symbol"],
                            address=function["address"],
                        ).content
                        (
                            destination / f"{function['address']:08X}-current.bin"
                        ).write_bytes(current)
                        emitted = [
                            e
                            for e in comparison.ranges
                            if e.address == function["address"]
                            and e.symbol == function["symbol"]
                        ]
                        exact = (
                            current
                            == function["original"]
                            == pre_images[member["selector"]]
                            and len(current) == 56
                        )
                        if not (
                            exact
                            and len(emitted) == 1
                            and emitted[0].source == source
                            and emitted[0].original_file_start == function["file_start"]
                            and emitted[0].section is None
                            and emitted[0].owners == (member["selector"],)
                            and emitted[0].content == current
                        ):
                            failures.append(member["selector"])
                        rows.append(
                            {
                                "selector": member["selector"],
                                "source": source,
                                "size": len(current),
                                "original_sha256": sha(function["original"]),
                                "current_sha256": sha(current),
                                "pre_sha256": sha(pre_images[member["selector"]]),
                                "complete_equal": exact,
                            }
                        )
                    save(destination / "actual-function-images.json", rows)
                    bind_report_inputs(report)
                    require(
                        report["target"] == batch["target"]
                        and report["source"] == source
                        and report["fresh_compilation"] is True
                        and report["member_count"] == len(selected)
                        and {r["selector"] for r in report["members"]}
                        == selected.keys()
                        and len(report["members"]) == len(selected)
                        and report["all_native_bytes_match"] is True
                        and not report["data_sections"]
                        and len(comparison.ranges) == len(selected)
                        and not failures
                        and report["source_image"]
                        == {
                            "source": source,
                            "kind": "current",
                            "sha256": outputs[source],
                            "size": states[source]["metadata"]["st_size"],
                        },
                        f"complete group images differ from PRE/original: {source}; {failures}",
                    )
                    require(
                        all(
                            r["original_size"] == r["current_size"] == 56
                            and r["byte_match"] is True
                            and r["flow_verified"] is True
                            for r in report["members"]
                        ),
                        "native member extents or control-flow proof differ",
                    )
                    verify_inputs()
                    summary["sources"].append(
                        {
                            "target": batch["target"],
                            "source": source,
                            "members": rows,
                            "comparison": (destination / "comparison.json")
                            .relative_to(ROOT)
                            .as_posix(),
                            "saved_elf": (destination / "current.linked.o")
                            .relative_to(ROOT)
                            .as_posix(),
                            "elf_sha256": sha(elf),
                        }
                    )
                    save(out / "summary.json", summary)
                    save(out / "input-states.json", states)
                    print(
                        f"{args.stage}: {source}: {len(rows)} complete 56B images equal PRE/original",
                        flush=True,
                    )
                verify_inputs()
        require(
            len(summary["sources"]) == 2
            and sum(len(s["members"]) for s in summary["sources"]) == 6,
            "incomplete stage capture",
        )
        summary.update(
            status="complete stage captures",
            member_count=6,
            complete_original_bytes=336,
            guards_and_writer_closed=True,
        )
        save(out / "summary.json", summary)
    except BaseException as error:
        summary.update(
            status="failed", error_type=type(error).__name__, error=str(error)
        )
        if isinstance(error, ProcessCleanupError):
            summary["cleanup"] = "unconfirmed; stop further native work and recovery"
            if error.observed is not None:
                save(out / "cleanup-failure.json", error.observed)
        save(out / "summary.json", summary)
        raise


if __name__ == "__main__":
    main()
