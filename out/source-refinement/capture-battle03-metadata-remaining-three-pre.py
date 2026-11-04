"""Capture the three remaining reviewed Battle03 PRE images without repairing metadata."""

from contextlib import ExitStack, closing
import hashlib
import json
from pathlib import Path

from harness.build.compilation import configured_command
from harness.build.compiler import COMPILER_INPUTS
from harness.build.dispatch import (
    close_dispatch, prepare_configured_dispatch, validate_dispatch,
)
from harness.build.dependencies import IncludeSnapshot
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.process import ProcessCleanupError, run_bounded
from harness.match.extraction import read_function_bytes, read_linked_image

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/source-refinement/battle03-metadata-remaining-three-pre"
PLAN = "out/source-refinement/battle03-metadata-remaining-three/plan.json"
PLAN_SHA = "a3db9175bc9e3d6cad27a5bb65e6505f54c47459824be5d05163a729904871f0"
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
        len(members) == 3 and sum(m["original_size"] for m in members) == 608,
        "Reviewed member closure differs",
    )
    OUT.mkdir()
    (OUT / "plan.json").write_bytes(data)
    summary = {
        "status": "running",
        "plan_sha256": PLAN_SHA,
        "members": [],
        "limits": "Unmodified-source PRE only. No source/layout/progress edits or POST acceptance. Git index observations are edit-safety evidence, excluded from compiler input guards.",
    }
    save("summary.json", summary)
    def retain_index(label):
        # Edit-safety evidence only; concurrent foreign staging is not a native input.
        try:
            with InputBatch(ROOT) as observation:
                state, content = observation.read(
                    ROOT / ".git/index", max_bytes=LIMIT,
                    include_metadata=True, single_link=True,
                )
            if content is not None:
                (OUT / f"index-{label}").write_bytes(content)
            return state
        except (OSError, ValueError) as error:
            return {"observation_error": f"{type(error).__name__}: {error}"}

    summary["index_before"] = retain_index("before")
    try:
        with acquire_writer(ROOT), InputBatch(ROOT) as inputs, ExitStack() as stack:
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
                if name == ".git/index":
                    continue
                bind(name)
                require(states[name] == expected, f"Reviewed input changed: {name}")
            require(sha(bind(PLAN)) == PLAN_SHA, "Reviewed plan changed")
            for member in members:
                prefix = f"{int(member['address'], 16):08X}"
                original_name = str(Path(PLAN).parent / f"{prefix}.original.bin")
                require(
                    sha(bind(original_name)) == member["original_sha256"],
                    "Reviewed original range changed",
                )
                assembly_name = str(Path(PLAN).parent / f"{prefix}.original.asm")
                (OUT / f"{prefix}.reviewed-original.asm").write_bytes(bind(assembly_name))
            bind(Path(__file__).relative_to(ROOT).as_posix())
            for name in COMPILER_INPUTS:
                require(bind(name) is not None, f"Missing compiler input: {name}")
            for tool in ("ld", "nm", "objcopy", "objdump"):
                require(
                    bind(f"toolchains/psyqo/bin/mipsel-none-elf-{tool}") is not None,
                    f"Missing comparison program: {tool}",
                )
            sources = [ROOT / m["source"] for m in members]
            dispatches = []
            forced = []
            for source in sources:
                output = ROOT / "build" / source.relative_to(ROOT).with_suffix(".o")
                dispatch = prepare_configured_dispatch(ROOT, source, output)
                stack.callback(close_dispatch, dispatch)
                dispatches.append(dispatch)
                forced.extend(Path(item["path"]) for item in dispatch.state["forced_inputs"])
            summary["configured_compiler_inputs"] = [
                {
                    "source": dispatch.source.relative_to(ROOT).as_posix(),
                    "contents": dispatch.state["contents"],
                    "programs": dispatch.programs.describe(),
                    "recipe": dispatch.state["recipe"],
                    "fingerprint": dispatch.fingerprint,
                }
                for dispatch in dispatches
            ]
            with closing(
                IncludeSnapshot(
                    ROOT, sources[0],
                    forced=tuple(dict.fromkeys([*sources[1:], *forced])),
                )
            ) as includes:

                def guard():
                    for name in list(states):
                        bind(name)
                    includes.validate()
                    for dispatch in dispatches:
                        validate_dispatch(dispatch)

                for name, expected in includes.describe_inputs().items():
                    bind(name)
                    require(states[name] == expected, f"Include changed: {name}")
                save("starting-input-states.json", states)
                for member in members:
                    guard()
                    arguments, _ = configured_command(
                        ROOT,
                        ROOT / member["source"],
                        ROOT
                        / "out/source-refinement/battle03-metadata-remaining-three/unused.o",
                        allow_grouped=True,
                    )
                    require(
                        arguments == member["configured_arguments"],
                        "Configured compiler arguments changed",
                    )
                    label = member["symbol"]
                    (OUT / f"{label}.source.c").write_bytes(bind(member["source"]))
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
                        == int(member["address"], 16)
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
                        address=int(member["address"], 16),
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
                    summary["members"].append(
                        {
                            "selector": member["selector"],
                            "source": member["source"],
                            "current_source_copy": str(OUT / f"{label}.source.c"),
                            "current_metadata_note": "Original split/detached metadata retained exactly; no attachment repair in PRE.",
                            "effective_settings": member["effective_settings"],
                            "ordered_arguments": arguments,
                            "elf_path": str(OUT / f"{label}.linked.o"),
                            "elf_sha256": sha(elf),
                            "original_sha256": sha(original),
                            "current_sha256": sha(current),
                            "report": report,
                        }
                    )
                    save("summary.json", summary)
                    # Retain actual reports/images even when an expected baseline differs.
                    if label == "shouldTriggerBattlerCountdownRetry":
                        require(
                            result["exit_code"] == 1
                            and report["status"] == "different"
                            and not report["byte_match"]
                            and report["current_size"] == 456
                            and report["instruction_count"] == {
                                "current": 114, "original": 114,
                                "matching": 100, "match_percent": 87.72,
                            },
                            "Historical partial baseline changed; inspect retained evidence",
                        )
                    else:
                        require(
                            result["exit_code"] == 0
                            and report["byte_match"] and report["exact_match"]
                            and current == original and len(current) == 76,
                            f"Exact complete baseline differs: {label}",
                        )
                    guard()
                guard()
                save("terminal-input-states.json", states)
        summary["index_after"] = retain_index("after")
        summary["index_changed_during_PRE"] = summary["index_before"] != summary["index_after"]
        summary.update(status="complete PRE captures", guards_and_writer_closed=True)
        save("summary.json", summary)
    except BaseException as error:
        summary.update(status="failed", error=f"{type(error).__name__}: {error}")
        if isinstance(error, ProcessCleanupError):
            summary["cleanup"] = "unconfirmed; stop native work"
        save("summary.json", summary)
        raise


if __name__ == "__main__":
    main()
