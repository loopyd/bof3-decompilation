"""Capture the promoted SHOP52B member against independently accepted PRE bytes."""
import argparse
from contextlib import closing
from dataclasses import asdict
import hashlib
import json
from pathlib import Path

from harness.build.compilation import configured_command
from harness.build.dependencies import IncludeSnapshot
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.process import ProcessCleanupError
from harness.domain.functions import parse_function_records
from harness.match.extraction import read_function_bytes, read_linked_image
from harness.match.images import capture_source_images
from harness.match.members import capture_members

ROOT = Path(__file__).resolve().parents[2]
BASE = "out/source-refinement/"
PLAN = BASE + "shop00-boundary-post-plan.json"
PRE = BASE + "shop00-boundary-pre/"
OUT = ROOT / (BASE + "shop00-boundary-post")
LIMIT = 64 * 1024 * 1024
def sha(content):
    return hashlib.sha256(content).hexdigest()


def require(ok, message):
    if not ok:
        raise ValueError(message)


def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def main():
    parser = argparse.ArgumentParser()
    for option in ("post-plan-sha256", "application-sha256", "pre-summary-sha256", "pre-index-sha256"):
        parser.add_argument("--" + option, required=True)
    args = parser.parse_args()
    require(Path.cwd() == ROOT and not OUT.exists(), "Run at root with fresh output")
    OUT.mkdir()
    summary = {"status": "running", "plan_sha256": args.post_plan_sha256, "accepted_pre_sha256": args.pre_summary_sha256, "members": [], "limits": "One complete52B strict native POST. Diagnostics=False provides raw bytes and actual ELF proof; no new rendered instruction metrics are claimed. Accepted PRE13-word metrics are preserved through complete raw equality. All143 normal TUs remain input/include/profile guarded; full normal build and public concern checks are separate future gates."}
    save(OUT / "summary.json", summary)
    try:
        with acquire_writer(ROOT), InputBatch(ROOT) as inputs:
            states = {}
            def bind(name):
                require(not Path(name).is_absolute() and ".." not in Path(name).parts, "Escaped input")
                state, content = inputs.read(ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True)
                require(name not in states or states[name] == state, f"Input changed: {name}")
                states[name] = state
                return content
            raw = bind(PLAN)
            require(sha(raw) == args.post_plan_sha256, "Applied plan changed")
            plan = json.loads(raw)
            for expected_states in (plan["current_input_states"], plan["foreign_states"]):
                for name, expected in expected_states.items():
                    bind(name)
                    require(states[name] == expected, f"Current/foreign state changed: {name}")
            app = json.loads(bind(plan["application_report"]))
            require(sha(bind(plan["application_report"])) == args.application_sha256 == plan["application_sha256"] and app["guards_and_writer_closed"] and app["status"] == "applied reviewed SHOP52B source/layout outputs; POST pending" and app["owned_post_sha256"] == plan["proposed_output_sha256"], "Application proof changed")
            for name, digest in ((app["pre_review"], app["pre_review_sha256"]), (app["foreign_report"], app["foreign_report_sha256"])):
                require(sha(bind(name)) == digest, "Accepted review/foreign report changed")
            pre = json.loads(bind(PRE + "summary.json"))
            require(sha(bind(PRE + "summary.json")) == args.pre_summary_sha256 == plan["accepted_pre_sha256"] == app["accepted_pre_sha256"] and pre["guards_and_writer_closed"], "Accepted PRE changed")
            index = bind(".git/index")
            require(sha(index) == args.pre_index_sha256 and states[".git/index"] == plan["runtime_index_state"] == pre["runtime_index_state"] == app["runtime_index_state"] and bind(PRE + "index-pre") == index, "Index changed")
            summary["runtime_index_state"] = states[".git/index"]
            (OUT / "index-post").write_bytes(index)
            (OUT / "plan.json").write_bytes(raw)
            bind(Path(__file__).resolve().relative_to(ROOT).as_posix())
            member, = plan["members"]
            source = member["source"]
            require([asdict(r) for r in parse_function_records(bind(source).decode())] == [member["record"]] and sha(bind(source)) == member["source_sha256"], "Strict metadata changed")
            for name, digest in plan["proposed_output_sha256"].items():
                require(sha(bind(name)) == digest and states[name] == app["after_states"][name], "Applied output changed")
            before, = pre["members"]
            original = bind(member["original_path"])
            pre_folder = PRE + member["symbol"] + "/"
            pre_elf = bind(before["saved_elf"])
            require(sha(pre_elf) == before["elf_sha256"] and read_function_bytes(pre_elf, function_name=member["symbol"], address=member["original_start"]).content == bind(pre_folder + "current.bin") == bind(pre_folder + "original.bin") == original and len(original) == 52 and sha(original) == member["original_sha256"] and before["raw_exact"], "Accepted complete PRE images changed")
            sources = [ROOT / u["source"] for u in plan["normal_build_units"]]
            require(len(sources) == len(set(sources)) == 143, "Normal TU closure changed")
            with closing(IncludeSnapshot(ROOT, sources[0], forced=tuple(sources[1:]))) as includes:
                require(includes.describe_inputs() == plan["include_input_states"], "Literal/negative include closure changed")
                for name, expected in includes.describe_inputs().items():
                    bind(name)
                    require(states[name] == expected, f"Include state changed: {name}")
                for unit in plan["normal_build_units"]:
                    argv, _ = configured_command(ROOT, ROOT / unit["source"], Path(unit["configured_arguments"][-1]), allow_grouped=True)
                    require(argv == unit["configured_arguments"], f"Normal ordered profile changed: {unit['source']}")
                admitted = capture_members(ROOT, source)
                function, = admitted["functions"]
                require(admitted["target"] == plan["target"] and not admitted["data_sections"] and not any(admitted["section_placements"].values()) and function["selector"] == member["selector"] and function["symbol"] == member["symbol"] and function["original"] == original and function["file_start"] == member["original_file_start"], "Strict boundary/binding changed")
                for name, expected in admitted["inputs"].items():
                    bind(name)
                    require((states[name] is None and expected is None) or states[name] is not None and expected is not None and states[name]["sha256"] == expected["sha256"], f"Admission hash differs: {name}")
                def guard():
                    includes.validate()
                    for name in list(states):
                        bind(name)
                guard()
                save(OUT / "starting-input-states.json", states)
                folder = OUT / member["symbol"]
                folder.mkdir()
                comparison = capture_source_images(ROOT, source, folder)
                report = comparison.report
                save(folder / "comparison.json", report)
                require(report["schema"] == "harness.byte-match-one/v1" and report["fresh_compilation"] and report["status"] == "exact_match" and report["byte_match"] and report["exact_match"] and report["size_delta"] == 0 and report["original_size"] == report["current_size"] == 52, "Native POST is not complete52B exact")
                outputs = report["native_outputs"]
                for name, expected in outputs.items():
                    content = bind(name)
                    require(states[name] == expected, f"Native output changed: {name}")
                    (folder / ("retained-" + Path(name).name)).write_bytes(content)
                linked, = [name for name in outputs if name.endswith(".linked.o")]
                elf = bind(linked)
                image = read_linked_image(elf, limit=LIMIT)
                functions = [s for s in image.symbols if s.info & 15 == 2 and s.section]
                require(len(functions) == 1 and functions[0].name == member["symbol"] and functions[0].size == 52 and not image.relocations, "Unexpected actual ELF inventory")
                emitted, = comparison.ranges
                current = read_function_bytes(elf, function_name=member["symbol"], address=member["original_start"]).content
                require(current == emitted.content == original and emitted.owners == (member["selector"],) and emitted.source == source and emitted.symbol == member["symbol"] and emitted.section is None and emitted.address == member["original_start"] and emitted.original_file_start == member["original_file_start"], "Actual emitted bytes/placement differ")
                for name, content in (("actual.elf", elf), ("original.bin", original), ("current.bin", current)):
                    (folder / name).write_bytes(content)
                    bind(str((folder / name).relative_to(ROOT)))
                summary["members"].append({"selector": member["selector"], "symbol": member["symbol"], "source": source, "size": 52, "raw_word_count": 13, "original_sha256": sha(original), "current_sha256": sha(current), "elf_path": str((folder / "actual.elf").relative_to(ROOT)), "elf_sha256": sha(elf), "saved_at": str(folder.relative_to(ROOT)), "record": member["record"], "configured_arguments": member["configured_arguments"], "effective_settings": member["effective_settings"], "preserved_pre_full_metrics": before["full_metrics"], "new_rendered_metrics": None})
                guard()
                save(OUT / "terminal-input-states.json", states)
        summary.update(status="complete POST captures; independent acceptance pending", member_count=1, complete_original_bytes=52, guards_and_writer_closed=True)
        save(OUT / "summary.json", summary)
    except BaseException as error:
        summary.update(status="failed; keep applied sources and fix cause", error=f"{type(error).__name__}: {error}")
        if isinstance(error, ProcessCleanupError):
            summary.update(cleanup="unconfirmed; stop native work", cleanup_observation=error.observed)
        save(OUT / "summary.json", summary)
        raise


if __name__ == "__main__":
    main()
