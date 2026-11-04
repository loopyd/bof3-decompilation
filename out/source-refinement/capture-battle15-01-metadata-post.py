"""Capture the eight reviewed Battle15 POST images against accepted exact and partial PRE results."""

from contextlib import closing
import hashlib
import argparse
from dataclasses import asdict
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
from harness.domain.functions import parse_function_records
from harness.build.compiler import load_compiler_configuration, resolve_compiler_settings

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/source-refinement/battle15-01-metadata-post"
PLAN = "out/source-refinement/battle15-01-metadata-post-plan.json"
PRE = "out/source-refinement/battle15-01-metadata-pre"
PRE_SHA = "a917dddacd2e4d3579d75ca1071dac03bbe7fe9086fec0f661aae7b9d749833d"
parser = argparse.ArgumentParser()
parser.add_argument("--plan-sha256", required=True)
parser.add_argument("--application-sha256", required=True)
args = parser.parse_args()
PLAN_SHA = args.plan_sha256
LIMIT = 64 * 1024 * 1024


def sha(content):
    return hashlib.sha256(content).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def save(name, value):
    (OUT / name).write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def main():
    require(Path.cwd() == ROOT and not OUT.exists(), "Run at root with fresh output")
    data = (ROOT / PLAN).read_bytes()
    require(sha(data) == PLAN_SHA, "Reviewed plan changed")
    plan = json.loads(data)
    members = plan["members"]
    require(
        len(members) == 8 and sum(m["original_size"] for m in members) == 740,
        "Reviewed member closure differs",
    )
    environment = dict(os.environ)
    prefix = Path("/home/koija/.codex/tmp/arg0-linux")
    environment["PATH"] = os.pathsep.join(entry for entry in environment["PATH"].split(os.pathsep) if not Path(entry).is_relative_to(prefix))
    environment["CMAKE_BUILD_PARALLEL_LEVEL"] = "1"
    OUT.mkdir()
    (OUT / "plan.json").write_bytes(data)
    summary = {
        "status": "running",
        "plan_sha256": PLAN_SHA,
        "members": [],
        "limits": "POST must preserve all eight complete PRE images and metrics, including the retained partial; no exact-only assertion or whole-target validation claim. Independent and final concern/build acceptance pending.",
    }
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
                require(states[name] == expected, f"Reviewed input changed: {name}")
            require(sha(bind(PLAN)) == PLAN_SHA, "Reviewed plan changed")
            bind(Path(__file__).resolve().relative_to(ROOT).as_posix())
            application_data = bind(plan["application_report"])
            require(sha(application_data) == args.application_sha256 == plan["application_sha256"], "Application report changed")
            application = json.loads(application_data)
            require(application["guards_and_writer_closed"] and application["accepted_pre_sha256"] == PRE_SHA == plan["accepted_pre_sha256"] and set(application["owned_outputs"]) == {m["source"] for m in members}, "Application/PRE closure differs")
            index = bind(".git/index")
            require(states[".git/index"] == application["index_before"] == application["index_after"], "Application index changed")
            (OUT / "index.post.bin").write_bytes(index)
            summary["runtime_index_state"] = states[".git/index"]
            pre_data = bind(PRE + "/summary.json")
            require(sha(pre_data) == PRE_SHA, "Accepted PRE summary changed")
            pre_summary = json.loads(pre_data)
            require(pre_summary["status"] == "complete PRE captures" and pre_summary["guards_and_writer_closed"] and pre_summary["member_count"] == 8 and pre_summary["complete_original_bytes"] == 740, "Accepted PRE incomplete")
            pre_rows = {m["selector"]: m for m in pre_summary["members"]}
            require(set(pre_rows) == {m["selector"] for m in members}, "PRE selector closure differs")
            pre_images = {}
            flags, compilers = load_compiler_configuration(ROOT)
            for member in members:
                name, symbol = member["source"], member["symbol"]
                body = bind(name)
                require(states[name] == application["after_states"][name] and sha(body) == member["proposed_sha256"], "Applied source changed")
                require(all(states[name]["metadata"][key] == application["before_states"][name]["metadata"][key] for key in ("st_dev", "st_ino", "st_mode", "st_nlink", "st_uid", "st_gid")), "Source inode changed")
                require([asdict(r) for r in parse_function_records(body.decode())] == [member["proposed_record"]], "Strict POST record changed")
                require(asdict(resolve_compiler_settings(name, body.decode(), flags, compilers)) == member["effective_settings"], "Effective POST profile changed")
                strict = capture_members(ROOT, name)
                require(strict["target"] == plan["target"] and len(strict["functions"]) == 1 and not strict["data_sections"] and not any(strict["section_placements"].values()), "Strict singleton/data closure differs")
                function = strict["functions"][0]
                require(function["selector"] == member["selector"] and function["symbol"] == symbol and function["file_start"] == member["boundary"]["file_start"] and len(function["original"]) == member["original_size"] and sha(function["original"]) == member["original_sha256"], "Reviewed POST identity/range differs")
                for path, expected in strict["inputs"].items():
                    bind(path)
                    require((expected is None and states[path] is None) or expected is not None and states[path] is not None and expected["sha256"] == states[path]["sha256"], "Strict POST input changed")
                original = bind(PRE + f"/{symbol}.original.bin")
                current = bind(PRE + f"/{symbol}.current.bin")
                elf = bind(PRE + f"/{symbol}.linked.o")
                before = pre_rows[member["selector"]]
                require(sha(elf) == before["elf_sha256"] and original == function["original"] and sha(original) == member["original_sha256"] and len(original) == member["original_size"], "Accepted PRE original/ELF changed")
                require(current == read_function_bytes(elf, function_name=symbol, address=member["boundary"]["virtual_start"]).content and sha(current) == before["report"]["function_image"]["current_sha256"] and len(current) == before["report"]["current_size"], "Accepted PRE current image changed")
                pre_images[member["selector"]] = (original, current)
                destination = OUT / "owned-post" / name
                destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_bytes(body)
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
                    object_path = (Path("build") / member["source"]).with_suffix(".o")
                    native = {}
                    for path in (object_path, Path(str(object_path) + ".s"), object_path.with_suffix(".linked.o")):
                        body = bind(path.as_posix())
                        if body is not None:
                            (retained / path.name).write_bytes(body)
                            native[path.as_posix()] = states[path.as_posix()]
                    (retained / "native-output-states.json").write_text(json.dumps(native, indent=2, sort_keys=True) + "\n")
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
                    before = pre_rows[member["selector"]]
                    pre_original, pre_current = pre_images[member["selector"]]
                    require(original == pre_original and current == pre_current, "Complete POST raw images differ from accepted PRE")
                    require(result["exit_code"] == before["actual_comparison_exit_code"], "POST process status differs from PRE")
                    for key in ("instruction_count", "first_mismatch", "original_size", "current_size", "size_delta", "byte_match", "exact_match", "function_image"):
                        require(report[key] == before["report"][key], f"Complete POST metric differs from PRE: {key}")
                    summary["members"].append(
                        {
                            "selector": member["selector"],
                            "elf_sha256": sha(elf),
                            "report": report,
                            "elf_path": str((OUT / f"{label}.linked.o").relative_to(ROOT)),
                            "original_path": str((OUT / f"{label}.original.bin").relative_to(ROOT)),
                            "current_path": str((OUT / f"{label}.current.bin").relative_to(ROOT)),
                            "complete_pre_images_and_metrics_equal": True,
                            "actual_comparison_exit_code": result["exit_code"],
                            "authored_progress_preserved": member["progress_tags"],
                            "current_size_equals_reviewed_range": len(current) == member["original_size"],
                            "raw_exact": current == original,
                        }
                    )
                    save("summary.json", summary)
                    for name in list(states):
                        bind(name)
                    includes.validate()
                save("terminal-input-states.json", states)
        summary.update(status="complete POST captures", guards_and_writer_closed=True, member_count=8, complete_original_bytes=740)
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
