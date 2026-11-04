"""Capture the eight reviewed Battle15 PRE images without repairing metadata."""

from contextlib import closing
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

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/source-refinement/battle15-01-metadata-pre"
PLAN = "out/source-refinement/metadata-battle15-01-ready-draft/plan.json"
PLAN_SHA = "73f934a7a5688d2272bd9f0094edf1f65f11f0651d600878b5971835cae4227d"
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
        "limits": "PRE measurements only. Preserve actual exact/partial and size metrics. No source/layout/progress edits or independent acceptance.",
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
            for member in members:
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
                    summary["members"].append(
                        {
                            "selector": member["selector"],
                            "elf_sha256": sha(elf),
                            "report": report,
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
        summary.update(status="complete PRE captures", guards_and_writer_closed=True, member_count=8, complete_original_bytes=740)
        save("summary.json", summary)
    except BaseException as error:
        summary.update(status="failed", error=f"{type(error).__name__}: {error}")
        if isinstance(error, ProcessCleanupError):
            summary["cleanup"] = "unconfirmed; stop native work"
        save("summary.json", summary)
        raise


if __name__ == "__main__":
    main()
