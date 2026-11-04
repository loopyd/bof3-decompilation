"""Capture all 26 WORKAREA_RESET POST members and preserve complete PRE images."""
from contextlib import closing
from dataclasses import asdict
import hashlib
import json
import os
from pathlib import Path

from harness.build.compilation import configured_command
from harness.build.compiler import COMPILER_INPUTS, load_compiler_configuration, resolve_compiler_settings
from harness.build.dependencies import IncludeSnapshot
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.process import ProcessCleanupError, run_bounded
from harness.domain.functions import parse_function_records
from harness.match.extraction import read_function_bytes, read_linked_image
from harness.match.members import capture_members

ROOT = Path(__file__).resolve().parents[2]
PLAN = "out/source-refinement/workarea-reset-post-plan.json"
PLAN_SHA = "022229e7e5d19ee3f1f1299010574dca73b64876dfa51df94335bb61a5e89444"
OUT = ROOT / "out/source-refinement/workarea-reset-post"
LIMIT = 64 * 1024 * 1024


def require(value, message):
    if not value:
        raise ValueError(message)


def sha(content):
    return hashlib.sha256(content).hexdigest()


def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def main():
    require(Path.cwd() == ROOT and not OUT.exists(), "Run from root with fresh output")
    data = (ROOT / PLAN).read_bytes()
    require(sha(data) == PLAN_SHA, "Reviewed plan changed")
    plan = json.loads(data)
    members = [{**m, "record": m.get("proposed_record", m["record"])} for m in plan["selected"] + plan["existing_consumers"]]
    require(len(members) == len({m["selector"] for m in members}) == len({m["source"] for m in members}) == 26 and len({m["target"] for m in members}) == 16 and all(m["original_size"] == 32 for m in members), "26-member/16-target scope changed")
    environment = dict(os.environ)
    prefix = Path("/home/koija/.codex/tmp/arg0-linux")
    environment["PATH"] = os.pathsep.join(entry for entry in environment["PATH"].split(os.pathsep) if not Path(entry).is_relative_to(prefix))
    OUT.mkdir()
    (OUT / "plan.json").write_bytes(data)
    summary = {"status": "running", "task": "3.23", "plan_sha256": PLAN_SHA, "members": [], "limits": "Complete POST preservation only; independent semantic, final-check and payload acceptance remain pending."}
    save(OUT / "summary.json", summary)
    try:
        with acquire_writer(ROOT), InputBatch(ROOT) as inputs:
            states = {}

            def bind(name, expected=None):
                state, content = inputs.read(ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True)
                require(name not in states or state == states[name], f"Input changed: {name}")
                require(expected is None or state == expected, f"Reviewed input changed: {name}")
                states[name] = state
                return content

            def verify():
                for name in list(states):
                    bind(name)
                includes.validate()

            for name, expected in plan["current_input_states"].items():
                content = bind(name)
                require(states[name] == expected, f"Reviewed input changed: {name}")
                if content is not None and name in plan["proposed_output_sha256"]:
                    saved = OUT / "owned-post" / name
                    saved.parent.mkdir(parents=True, exist_ok=True)
                    saved.write_bytes(content)
            require(sha(bind(PLAN)) == PLAN_SHA, "Reviewed plan changed")
            pre_data = bind("out/source-refinement/workarea-reset-pre/summary.json")
            require(sha(pre_data) == "71db518bc8a0f750991b5c186bb58b473d5176cee654ed6f82885990e230b6ef", "Accepted PRE summary changed")
            pre = json.loads(pre_data)
            require(pre["status"] == "complete PRE captures" and pre["member_count"] == 26 and pre["complete_original_bytes"] == 832 and pre["guards_and_writer_closed"], "Accepted PRE incomplete")
            pre_rows = {row["selector"]: row for row in pre["members"]}
            require(set(pre_rows) == {m["selector"] for m in members}, "PRE closure differs")
            bind(Path(__file__).resolve().relative_to(ROOT).as_posix())
            for name in COMPILER_INPUTS:
                bind(name)
            sources = [ROOT / m["source"] for m in members]
            with closing(IncludeSnapshot(ROOT, sources[0], forced=tuple(sources[1:]))) as includes:
                for name, expected in includes.describe_inputs().items():
                    bind(name, expected)
                    require(states[name] == expected, f"Include changed: {name}")
                flags, compilers = load_compiler_configuration(ROOT)
                prepared = []
                for member in members:
                    source = member["source"]
                    text = bind(source).decode()
                    require([asdict(r) for r in parse_function_records(text)] == [member["record"]], f"Metadata/body attachment changed: {source}")
                    profile = asdict(resolve_compiler_settings(source, text, flags, compilers))
                    arguments, _ = configured_command(ROOT, ROOT / source, Path(member["configured_arguments"][-1]), allow_grouped=True)
                    require(profile == member["effective_settings"] and arguments == member["configured_arguments"], f"Configured profile changed: {source}")
                    captured = capture_members(ROOT, source)
                    require(captured["target"] == member["target"] and len(captured["functions"]) == 1 and not captured["data_sections"] and not any(captured["section_placements"].values()), f"Strict singleton/data closure changed: {source}")
                    function = captured["functions"][0]
                    original = bind(member["original_path"])
                    require(function["selector"] == member["selector"] and function["symbol"] == member["symbol"] and function["address"] == member["record"]["address"] and function["original"] == original and len(original) == 32 and sha(original) == member["original_sha256"], f"Original identity/range changed: {source}")
                    for name, expected in captured["inputs"].items():
                        bind(name)
                        require(states[name] is not None and states[name]["sha256"] == expected["sha256"], f"Member input changed: {name}")
                    saved_pre = pre_rows[member["selector"]]
                    pre_root = saved_pre["saved_at"]
                    pre_elf = bind(pre_root + "/" + Path(source).stem + ".linked.o")
                    require(sha(pre_elf) == saved_pre["elf_sha256"], "Accepted PRE ELF changed")
                    pre_image = read_function_bytes(pre_elf, function_name=member["symbol"], address=member["record"]["address"]).content
                    require(pre_image == original == bind(pre_root + "/current.bin") == bind(pre_root + "/original.bin"), "Accepted complete PRE bytes changed")
                    prepared.append((member, original, profile, arguments))
                verify()
                save(OUT / "starting-input-states.json", states)
                for member, original, profile, arguments in prepared:
                    verify()
                    destination = OUT / (member["target"].replace("/", "--") + f"-{member['record']['address']:08X}")
                    destination.mkdir()
                    (destination / "original.bin").write_bytes(original)
                    save(destination / "metadata-profile.json", {"record": member["record"], "profile": profile, "configured_arguments": arguments})
                    argv = ["bin/harness", "lift", "asm-diff", member["selector"], "--json", "--detail", "full", "--image-evidence"]
                    print(" ".join(argv), flush=True)
                    with (destination / "stdout").open("wb") as stdout, (destination / "stderr").open("wb") as stderr:
                        result = run_bounded(ROOT, argv, env=environment, output_limit=LIMIT, on_output=lambda stream, chunk: (stdout if stream == "stdout" else stderr).write(chunk))
                    save(destination / "process.json", result)
                    native = {}
                    object_path = (Path("build") / member["source"]).with_suffix(".o")
                    for output in (object_path, Path(str(object_path) + ".s"), object_path.with_suffix(".linked.o")):
                        content = bind(output.as_posix())
                        if content is not None:
                            (destination / output.name).write_bytes(content)
                            native[output.as_posix()] = states[output.as_posix()]
                    save(destination / "native-output-states.json", native)
                    require(not result["failure"] and result["exit_code"] in (0, 1), f"Public comparison failed: {member['selector']}")
                    report = json.loads(result["stdout"])
                    save(destination / "comparison.json", report)
                    elf = (destination / object_path.with_suffix(".linked.o").name).read_bytes()
                    image = read_linked_image(elf, limit=LIMIT)
                    functions = [s for s in image.symbols if s.info & 15 == 2 and s.section]
                    current = read_function_bytes(elf, function_name=member["symbol"], address=member["record"]["address"]).content
                    (destination / "current.bin").write_bytes(current)
                    for key in ("original", "current", "linked", "original_bytes", "diff", "compiler", "build_log"):
                        output = Path(report["outputs"][key])
                        require(output.is_absolute() and output.is_relative_to(ROOT), "Public artifact escaped root")
                        (destination / ("public-" + output.name)).write_bytes(bind(output.relative_to(ROOT).as_posix()))
                    require(report["schema"] == "harness.asm-diff-one/v2" and report["function"] == member["symbol"] and report["source"] == str(ROOT / member["source"]) and int(report["address"], 16) == member["record"]["address"] and report["original_size"] == report["current_size"] == 32, "Public complete identity/range differs")
                    require(len(functions) == 1 and functions[0].name == member["symbol"] and not image.relocations and current == original and report["function_image"] == {"original_sha256": sha(original), "current_sha256": sha(current)}, "Actual singleton ELF image differs")
                    require(result["exit_code"] == 0 and report["byte_match"] is True and report["exact_match"] is True and report["first_mismatch"] is None and report["instruction_count"] == {"original": 8, "current": 8, "matching": 8, "match_percent": 100.0}, "Complete byte/native instruction proof differs")
                    verify()
                    summary["members"].append({"selector": member["selector"], "source": member["source"], "saved_at": str(destination.relative_to(ROOT)), "elf_sha256": sha(elf), "original_sha256": sha(original), "current_sha256": sha(current), "size": len(current), "instruction_count": report["instruction_count"]})
                    save(OUT / "summary.json", summary)
                verify()
                save(OUT / "terminal-input-states.json", states)
        require(len(summary["members"]) == 26, "Incomplete POST closure")
        summary.update(status="complete POST captures", member_count=26, complete_original_bytes=832, guards_and_writer_closed=True)
        save(OUT / "summary.json", summary)
    except BaseException as error:
        summary.update(status="failed", error=f"{type(error).__name__}: {error}")
        if isinstance(error, ProcessCleanupError):
            summary["cleanup"] = "unconfirmed; stop native work"
            if error.observed is not None:
                save(OUT / "cleanup-failure.json", error.observed)
        save(OUT / "summary.json", summary)
        raise


if __name__ == "__main__":
    main()
