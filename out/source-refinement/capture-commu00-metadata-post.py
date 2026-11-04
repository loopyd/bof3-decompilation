"""Capture the reviewed COMMU00 POST image without repairing metadata."""

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

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/source-refinement/commu00-metadata-post"
PLAN = "out/source-refinement/commu00-metadata-post-plan.json"
parser = argparse.ArgumentParser()
parser.add_argument("--plan-sha256", required=True)
PLAN_SHA = parser.parse_args().plan_sha256
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
        len(members) == 1 and sum(m["original_size"] for m in members) == 64,
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
        "limits": "POST preservation capture. No independent or payload acceptance.",
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
                    text=bind(member["source"]).decode()
                    require([asdict(r) for r in parse_function_records(text)] == [member["proposed_record"]], "POST record differs")
                    captured=capture_members(ROOT, member["source"])
                    require(captured["target"]==plan["target"] and len(captured["functions"])==1 and not captured["data_sections"] and not any(captured["section_placements"].values()), "POST strict closure differs")
                    function=captured["functions"][0]
                    require(function["selector"]==member["selector"] and function["symbol"]==member["symbol"] and len(function["original"])==64 and sha(function["original"])==member["original_sha256"], "POST original range differs")
                    for name,expected in captured["inputs"].items():
                        bind(name)
                        require(states[name] is not None and states[name]["sha256"]==expected["sha256"], "Strict POST input differs")
                    pre_root=ROOT/"out/source-refinement/commu00-metadata-pre"
                    pre_original=bind((pre_root/f"{member['symbol']}.original.bin").relative_to(ROOT).as_posix())
                    pre_current=bind((pre_root/f"{member['symbol']}.current.bin").relative_to(ROOT).as_posix())
                    pre_elf=bind((pre_root/f"{member['symbol']}.linked.o").relative_to(ROOT).as_posix())
                    pre_extracted=read_function_bytes(pre_elf,function_name=member["symbol"],address=member["boundary"]["virtual_start"]).content
                    require(function["original"]==pre_original==pre_current==pre_extracted, "Accepted PRE images changed")
                    label = member["symbol"]
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
                    require(
                        current == original and len(current) == 64
                        and result["exit_code"] == 0
                        and report["byte_match"] is True and report["exact_match"] is True
                        and report["first_mismatch"] is None
                        and report["instruction_count"] == {"original":16,"current":16,"matching":16,"match_percent":100.0},
                        "Complete original POST differs; retain the failure",
                    )
                    for key in ("original", "current", "linked", "original_bytes", "diff", "compiler", "build_log"):
                        artifact=Path(report["outputs"][key]);require(artifact.is_absolute() and artifact.is_relative_to(ROOT), "Escaped native output")
                        (OUT / ("public-"+artifact.name)).write_bytes(bind(artifact.relative_to(ROOT).as_posix()))
                    summary["members"].append(
                        {
                            "selector": member["selector"],
                            "elf_sha256": sha(elf),
                            "report": report,
                        }
                    )
                    save("summary.json", summary)
                    for name in list(states):
                        bind(name)
                    includes.validate()
                save("terminal-input-states.json", states)
        summary.update(status="complete POST captures", guards_and_writer_closed=True, member_count=1, complete_original_bytes=64)
        save("summary.json", summary)
    except BaseException as error:
        summary.update(status="failed", error=f"{type(error).__name__}: {error}")
        if isinstance(error, ProcessCleanupError):
            summary["cleanup"] = "unconfirmed; stop native work"
        save("summary.json", summary)
        raise


if __name__ == "__main__":
    main()
