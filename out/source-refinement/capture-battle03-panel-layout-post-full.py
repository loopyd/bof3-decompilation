"""Compare two applied panel boundaries through complete public full-diagnostic reports."""

from contextlib import closing
import hashlib
import json
from pathlib import Path

from harness.build.compilation import configured_command
from harness.build.dependencies import IncludeSnapshot
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.process import ProcessCleanupError, run_bounded
from harness.match.extraction import read_function_bytes, read_linked_image

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/source-refinement/battle03-panel-layout-post-full"
PLAN = "out/source-refinement/battle03-panel-layout-post-full-plan.json"
PLAN_SHA = "cab8d462a1cc3f4f0efba900104eb7647e5d5ed3d823442e9fe2080323ccf5c3"
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
        len(members) == 2 and sum(m["original_size"] for m in members) == 108,
        "Reviewed member closure differs",
    )
    OUT.mkdir()
    (OUT / "plan.json").write_bytes(data)
    summary = {
        "status": "running",
        "plan_sha256": PLAN_SHA,
        "members": [],
        "limits": "Complete POST preservation capture only. Independent semantic, build and payload acceptance remains pending.",
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
                        ROOT
                        / member["profile"]["ordered_arguments"][-1],
                        allow_grouped=True,
                    )
                    require(
                        arguments == member["profile"]["ordered_arguments"],
                        "Configured compiler arguments changed",
                    )
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
                            ROOT, argv, output_limit=LIMIT, on_output=retain
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
                    elf = linked.read_bytes()
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
                    original = Path(report["outputs"]["original_bytes"]).read_bytes()
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
                    baseline = member["baseline_report"]
                    for key in ("function_image", "instruction_count", "current_size", "original_size", "first_mismatch", "byte_match", "exact_match"):
                        require(report.get(key) == baseline.get(key), f"PRE/POST {key} differs: {label}")
                    require(current == (ROOT / member["baseline_directory"] / f"{label}.current.bin").read_bytes(), f"Complete PRE current image differs: {label}")
                    require(original == (ROOT / member["baseline_directory"] / f"{label}.original.bin").read_bytes(), f"Complete PRE original differs: {label}")
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
        summary.update(status="complete POST preservation captures", guards_and_writer_closed=True)
        save("summary.json", summary)
    except BaseException as error:
        summary.update(status="failed", error=f"{type(error).__name__}: {error}")
        if isinstance(error, ProcessCleanupError):
            summary["cleanup"] = "unconfirmed; stop native work"
        save("summary.json", summary)
        raise


if __name__ == "__main__":
    main()
