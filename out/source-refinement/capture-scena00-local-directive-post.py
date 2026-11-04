"""Capture two unchanged Scena00 POST images after the reviewed parser repair."""

from contextlib import closing
from dataclasses import asdict
import argparse
import hashlib
import json
import os
from pathlib import Path

from harness.build.compilation import configured_command
from harness.build.dependencies import IncludeSnapshot
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.process import ProcessCleanupError, run_bounded
from harness.match.extraction import read_function_bytes, read_linked_image
from harness.match.members import capture_members
from harness.domain.functions import parse_function_records, requires_function_expansion

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/source-refinement/scena00-local-directive-post"
PRE = "out/source-refinement/scena00-local-directive-pre"
PRE_SHA = "530ae415162b6eafdffdf22f607813b69fadbed9cb74d802901c9c2f2dd98a12"
CODE_PLAN = "out/source-refinement/scena00-local-directive-code-draft/plan.json"
CODE_SHA = "ad66d6c8a53df56d53bc87529fb5373f83073ba813f8c49f7aa01a8e85cb9be5"
PLAN = "out/source-refinement/scena00-local-directive-pre-plan/plan.json"
PLAN_SHA = "c11207dfdfd7aa59475a2a7dbe1999bb80ab95619b369129ad8b839d0aad6924"
LIMIT = 64 * 1024 * 1024


def sha(content):
    return hashlib.sha256(content).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def save(name, value):
    (OUT / name).write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--application-report", required=True)
    parser.add_argument("--application-sha256", required=True)
    args = parser.parse_args()
    application_path = Path(args.application_report)
    require(not application_path.is_absolute() and ".." not in application_path.parts, "Application path must be repository-relative")
    require(Path.cwd() == ROOT and not OUT.exists(), "Run at root with fresh output")
    data = (ROOT / PLAN).read_bytes()
    require(sha(data) == PLAN_SHA, "Reviewed plan changed")
    plan = json.loads(data)
    code_data = (ROOT / CODE_PLAN).read_bytes()
    require(sha(code_data) == CODE_SHA, "Reviewed code plan changed")
    code = json.loads(code_data)
    changed = set(code["paths"])
    require(changed == {"tools/python/harness/domain/functions.py", "tools/python/harness/build/translation.py", "tools/python/harness/naming/source_identifiers.py"}, "Reviewed code scope changed")
    members = plan["members"]
    require(
        len(members) == 2 and sum(m["original_size"] for m in members) == 1972,
        "Reviewed member closure differs",
    )
    environment = dict(os.environ)
    environment["CMAKE_BUILD_PARALLEL_LEVEL"] = "1"
    OUT.mkdir()
    (OUT / "plan.json").write_bytes(data)
    summary = {
        "status": "running",
        "plan_sha256": PLAN_SHA,
        "code_plan_sha256": CODE_SHA,
        "application_report": args.application_report,
        "application_sha256": args.application_sha256,
        "pre_summary_sha256": PRE_SHA,
        "members": [],
        "limits": "POST of two unchanged source bodies after only3reviewed code changes. Public configured compilation exercises existing expansion and assembly-closure owners; no duplicate CPP. Preserve complete accepted PRE raw images and metrics including1072original/1076current partial. WholeELFhash equality is not required. No normalbuild/Splat/source-validation in this phase. Git index copied separately for edit safety, not a compiler input guard.",
    }
    index = (ROOT / ".git/index").read_bytes()
    (OUT / "index-pre").write_bytes(index)
    summary["runtime_index_sha256"] = sha(index)
    save("summary.json", summary)
    try:
        with acquire_writer(ROOT), InputBatch(ROOT) as inputs:
            states = {}

            def bind(name, expected=None):
                state, content = inputs.read(
                    ROOT / name,
                    max_bytes=LIMIT,
                    include_metadata=True,
                    single_link=True,
                )
                if name in states:
                    require(state == states[name], f"Input changed: {name}")
                if expected is not None:
                    require(state == expected, f"Reviewed input changed: {name}")
                states[name] = state
                return content

            for name, expected in plan["current_input_states"].items():
                bind(name)
                if name in changed:
                    require(expected["sha256"] == code["original_sha256"][name], "Code predecessor differs")
                    require(states[name]["sha256"] == code["proposed_output_sha256"][name], "Applied code differs")
                else:
                    require(states[name] == expected, f"Unchanged PRE input changed: {name}")
            require(sha(bind(CODE_PLAN)) == CODE_SHA, "Reviewed code plan changed")
            require(sha(bind(args.application_report)) == args.application_sha256, "Application report changed")
            for name in changed:
                require(sha(bind(name)) == code["proposed_output_sha256"][name], "Applied code output changed")
                proposal = "out/source-refinement/scena00-local-directive-code-draft/files/" + name
                require(bind(name) == bind(proposal), "Applied code differs from reviewed proposal")
            pre = json.loads(bind(PRE + "/summary.json"))
            require(sha(bind(PRE + "/summary.json")) == PRE_SHA and pre["status"] == "complete PRE captures" and pre["guards_and_writer_closed"] and pre["member_count"] == 2 and pre["complete_original_bytes"] == 1972, "Accepted PRE changed")
            previous = {row["selector"]: row for row in pre["members"]}
            require(set(previous) == {m["selector"] for m in members}, "PRE closure differs")
            for member in members:
                label = member["symbol"]
                before = previous[member["selector"]]
                old_elf = bind(PRE + f"/{label}.linked.o")
                old_current = bind(PRE + f"/{label}.current.bin")
                old_original = bind(PRE + f"/{label}.original.bin")
                require(sha(old_elf) == before["elf_sha256"] and sha(old_current) == before["report"]["function_image"]["current_sha256"] and sha(old_original) == member["original_sha256"], "Saved PRE images changed")
                require(read_function_bytes(old_elf, function_name=label, address=member["boundary"]["virtual_start"]).content == old_current, "Saved PRE ELF function image differs")
                record, = parse_function_records(bind(member["source"]).decode())
                require(record.kind == "function" and record.spelling == label and record.address == member["boundary"]["virtual_start"] and requires_function_expansion(bind(member["source"]).decode()), "Expected ordinary configured expansion admission")
                admitted = capture_members(ROOT, member["source"])
                require(len(admitted["functions"]) == 1, "Strict member closure differs")
                admitted_member = admitted["functions"][0]
                require(admitted_member["selector"] == member["selector"] and admitted_member["symbol"] == label and admitted_member["original"] == old_original, "Strict original identity differs")
                for name, expected in admitted["inputs"].items():
                    bind(name)
                    actual = states[name]
                    require(
                        actual is None if expected is None else actual is not None and {"sha256": actual["sha256"]} == expected,
                        "Strict member input changed",
                    )
                save(label + ".ordinary-admission.json", {"record": asdict(record), "kind": "function", "requires_configured_expansion": True, "selector": member["selector"], "symbol": label, "source": member["source"], "original_sha256": sha(admitted_member["original"]), "inputs": admitted["inputs"], "naming_fact_provenance": "Parent may separately inspect existing naming facts; this report does not invoke naming or extra CPP."})
            require(sha(bind(PLAN)) == PLAN_SHA, "Reviewed plan changed")
            bind(Path(__file__).resolve().relative_to(ROOT).as_posix())
            for member in members:
                require(sha(bind(member["source"])) == member["current_sha256"], "Current source changed")
                require(sha(bind(member["original_path"])) == member["original_sha256"], "Saved original changed")
                destination = OUT / "owned-pre" / member["source"]
                destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_bytes(bind(member["source"]))
            sources = [ROOT / m["source"] for m in members]
            with closing(
                IncludeSnapshot(ROOT, sources[0], forced=tuple(sources[1:]))
            ) as includes:
                for name, expected in includes.describe_inputs().items():
                    bind(name)
                    require(states[name] == expected, f"Include changed: {name}")
                save("starting-input-states.json", states)
                for member in members:
                    for name in list(states):
                        bind(name)
                    includes.validate()
                    arguments, _ = configured_command(
                        ROOT,
                        ROOT / member["source"],
                        Path(member["configured_arguments"][-1]),
                        allow_grouped=True,
                    )
                    require(
                        arguments == member["configured_arguments"],
                        "Configured compiler arguments changed",
                    )
                    label = member["symbol"]
                    retained = OUT / label
                    retained.mkdir()
                    argv = [
                        "bin/harness",
                        "lift",
                        "asm-diff",
                        member["selector"],
                        "--json",
                        "--detail",
                        "full",
                        "--image-evidence",
                    ]
                    print(" ".join(argv), flush=True)
                    with (
                        (OUT / f"{label}.stdout").open("wb") as stdout,
                        (OUT / f"{label}.stderr").open("wb") as stderr,
                    ):

                        def retain(stream, chunk):
                            (stdout if stream == "stdout" else stderr).write(chunk)

                        result = run_bounded(
                            ROOT, argv, env=environment, output_limit=LIMIT, on_output=retain
                        )
                    save(f"{label}.process.json", result)
                    try:
                        available = json.loads(result["stdout"])
                    except (ValueError, TypeError):
                        available = {}
                    for key, name in available.get("outputs", {}).items():
                        if not isinstance(name, str):
                            continue
                        artifact = Path(name)
                        if artifact.is_absolute() and artifact.is_relative_to(ROOT) and artifact.is_file():
                            (retained / (key + "-" + artifact.name)).write_bytes(bind(artifact.relative_to(ROOT).as_posix()))
                    require(
                        not result["failure"] and result["exit_code"] in (0, 1),
                        f"Public comparison failed: {label}",
                    )
                    report = json.loads(result["stdout"])
                    require(
                        report["schema"] == "harness.asm-diff-one/v2"
                        and report["function"] == label
                        and int(report["address"], 16)
                        == member["boundary"]["virtual_start"]
                        and report["source"] == str(ROOT / member["source"])
                        and report["original_size"] == member["original_size"]
                        and report["function_image"]["original_sha256"]
                        == member["original_sha256"],
                        f"Complete reviewed identity/range differs: {label}",
                    )
                    linked = Path(report["current_object"]).with_suffix(".linked.o")
                    elf = bind(linked.relative_to(ROOT).as_posix())
                    image = read_linked_image(elf, limit=LIMIT)
                    functions = [
                        s for s in image.symbols if s.info & 15 == 2 and s.section
                    ]
                    require(
                        len(functions) == 1
                        and functions[0].name == label
                        and not image.relocations,
                        f"Unexpected ELF closure: {label}",
                    )
                    current = read_function_bytes(
                        elf,
                        function_name=label,
                        address=member["boundary"]["virtual_start"],
                    ).content
                    original = bind(Path(report["outputs"]["original_bytes"]).relative_to(ROOT).as_posix())
                    require(
                        len(original) == member["original_size"]
                        and original == bind(member["original_path"])
                        and sha(original) == member["original_sha256"]
                        and len(current) == report["current_size"]
                        and sha(current) == report["function_image"]["current_sha256"],
                        f"Actual complete image differs: {label}",
                    )
                    (OUT / f"{label}.linked.o").write_bytes(elf)
                    (OUT / f"{label}.original.bin").write_bytes(original)
                    (OUT / f"{label}.current.bin").write_bytes(current)
                    for key in ("original", "current", "linked", "original_bytes", "diff", "compiler", "build_log"):
                        artifact=Path(report["outputs"][key]);require(artifact.is_absolute() and artifact.is_relative_to(ROOT), "Escaped native output")
                        (retained / ("public-"+artifact.name)).write_bytes(bind(artifact.relative_to(ROOT).as_posix()))
                    require(report["instruction_count"]["original"] > 0 and report["instruction_count"]["current"] > 0, "Missing complete instruction metrics")
                    before = previous[member["selector"]]
                    require(result["exit_code"] == before["actual_comparison_exit_code"], "Comparison lifecycle changed")
                    for key in ("instruction_count", "first_mismatch", "original_size", "current_size", "size_delta", "byte_match", "exact_match", "function_image"):
                        require(report[key] == before["report"][key], f"Accepted PRE metric changed: {label}:{key}")
                    require(current == bind(PRE + f"/{label}.current.bin") and original == bind(PRE + f"/{label}.original.bin"), "Complete PRE raw images differ")
                    summary["members"].append(
                        {
                            "selector": member["selector"],
                            "elf_sha256": sha(elf),
                            "report": report,
                            "actual_comparison_exit_code": result["exit_code"],
                            "authored_progress_preserved": member["progress_tags"],
                            "current_size_equals_reviewed_range": len(current) == member["original_size"],
                            "raw_exact": current == original,
                            "complete_pre_images_and_metrics_equal": True,
                            "ordinary_configured_expansion_admitted": True,
                        }
                    )
                    save("summary.json", summary)
                    for name in list(states):
                        bind(name)
                    includes.validate()
                save("terminal-input-states.json", states)
        summary.update(status="complete POST captures", guards_and_writer_closed=True, member_count=2, complete_original_bytes=1972)
        save("summary.json", summary)
    except BaseException as error:
        summary.update(status="failed", error=f"{type(error).__name__}: {error}")
        if isinstance(error, ProcessCleanupError):
            summary["cleanup"] = "unconfirmed; stop native work"
            summary["cleanup_observation"] = error.observed
        save("summary.json", summary)
        raise


if __name__ == "__main__":
    main()
