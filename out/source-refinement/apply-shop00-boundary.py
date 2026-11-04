"""Apply only the two reviewed SHOP52B outputs after independently accepted PRE."""
import argparse
from dataclasses import asdict
import hashlib
import json
from pathlib import Path

from harness.build.compilation import configured_command
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.domain.functions import parse_function_records
from harness.match.members import capture_members

ROOT = Path(__file__).resolve().parents[2]
BASE = "out/source-refinement/"
DRAFT = BASE + "shop00-boundary-draft/"
PLAN = DRAFT + "plan.json"
PLAN_SHA = "bc371844d3abab4acaae2f54de5f2a66c4a7ce70b498d1d51d4034d5237f9bf1"
PRE = BASE + "shop00-boundary-pre/"
OUT = ROOT / (BASE + "shop00-boundary-application")
POST_PLAN = ROOT / (BASE + "shop00-boundary-post-plan.json")
LIMIT = 64 * 1024 * 1024
def sha(content):
    return hashlib.sha256(content).hexdigest()


def require(ok, message):
    if not ok:
        raise ValueError(message)


def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def read(batch, name):
    require(not Path(name).is_absolute() and ".." not in Path(name).parts, "Escaped input")
    return batch.read(ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--pre-summary-sha256", required=True)
    parser.add_argument("--pre-index-sha256", required=True)
    parser.add_argument("--pre-review", required=True)
    parser.add_argument("--pre-review-sha256", required=True)
    parser.add_argument("--foreign-report", required=True)
    parser.add_argument("--foreign-report-sha256", required=True)
    args = parser.parse_args()
    require(Path.cwd() == ROOT and not OUT.exists() and not POST_PLAN.exists(), "Run at root with fresh outputs")
    report = {"status": "running", "plan_sha256": PLAN_SHA, "accepted_pre_sha256": args.pre_summary_sha256, "edits": []}
    try:
        with acquire_writer(ROOT):
            with InputBatch(ROOT) as before_batch:
                states = {}
                def bind(name):
                    state, body = read(before_batch, name)
                    require(name not in states or states[name] == state, f"Input changed: {name}")
                    states[name] = state
                    return body
                plan = json.loads(bind(PLAN))
                require(sha(bind(PLAN)) == PLAN_SHA, "Proposal changed")
                pre = json.loads(bind(PRE + "summary.json"))
                require(sha(bind(PRE + "summary.json")) == args.pre_summary_sha256, "PRE changed")
                require(pre["plan_sha256"] == PLAN_SHA and pre["guards_and_writer_closed"] and pre["member_count"] == 1 and pre["complete_original_bytes"] == 52 and pre["status"] == "complete PRE captures; independent acceptance pending", "PRE incomplete")
                proof = pre["members"][0]
                require(proof["raw_exact"] and proof["actual_comparison_exit_code"] == 0 and proof["current_size"] == 52 and proof["full_metrics"]["instruction_count"] == {"original": 13, "current": 13, "matching": 13, "match_percent": 100.0} and proof["full_metrics"]["first_mismatch"] is None, "Fresh complete52B exact PRE required")
                review = json.loads(bind(args.pre_review))
                require(sha(bind(args.pre_review)) == args.pre_review_sha256 and review["summary_sha256"] == args.pre_summary_sha256 and review["decision"] == "SHOP00 PRE accepted" and review["material_findings"] == [], "Independent accepted PRE review required")
                foreign = json.loads(bind(args.foreign_report))
                require(sha(bind(args.foreign_report)) == args.foreign_report_sha256, "Foreign preservation baseline changed")
                foreign_states = foreign["foreign_tracked_states"]
                require(not set(foreign_states) & set(plan["owned_paths"]), "Foreign/owned overlap")
                terminal = json.loads(bind(PRE + "terminal-input-states.json"))
                for expected_states in (plan["current_input_states"], terminal, foreign_states):
                    for name, expected in expected_states.items():
                        bind(name)
                        require(states[name] == expected, f"PRE/current/foreign state changed: {name}")
                index = bind(".git/index")
                require(sha(index) == args.pre_index_sha256 and states[".git/index"] == pre["runtime_index_state"] and bind(PRE + "index-pre") == index, "PRE index changed")
                bind(Path(__file__).resolve().relative_to(ROOT).as_posix())
                member = plan["members"][0]
                saved = PRE + member["symbol"] + "/"
                require(bind(saved + "current.bin") == bind(saved + "original.bin") == bind(member["original_path"]) and sha(bind(saved + "linked.o")) == proof["elf_sha256"], "Saved actual PRE artifacts changed")
                outputs = {name: bind(DRAFT + "proposed/" + name) for name in plan["proposed_output_sha256"]}
                require(set(outputs) == set(plan["owned_paths"]) and len(outputs) == 2, "Two-output closure changed")
                original = {name: (states[name], bind(name)) for name in outputs}
                for name, body in outputs.items():
                    require(sha(body) == plan["proposed_output_sha256"][name] and sha(original[name][1]) == plan["owned_pre_sha256"][name] and bind(DRAFT + "owned-pre/" + name) == original[name][1], f"Owned proposal changed: {name}")
                old_text = original[member["source"]][1].decode()
                new_text = outputs[member["source"]].decode()
                old_record, = parse_function_records(old_text)
                new_record, = parse_function_records(new_text)
                require(old_text[old_record.implementation_start:] == new_text[new_record.implementation_start:] and old_record.address == new_record.address and old_record.spelling == new_record.spelling, "Body/ABI changed")
            OUT.mkdir()
            (OUT / "index-pre").write_bytes(index)
            save(OUT / "report.json", report)
            for name, (_, body) in original.items():
                path = OUT / "owned-pre" / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(body)
            with InputBatch(ROOT) as stable:
                for name, expected in states.items():
                    if name not in outputs:
                        require(read(stable, name)[0] == expected, f"Stable input changed: {name}")
                with InputBatch(ROOT) as mutable:
                    for name, expected in original.items():
                        require(read(mutable, name) == expected, f"Owned predecessor changed: {name}")
                for name, body in outputs.items():
                    require((ROOT / name).read_bytes() == original[name][1], "Owned predecessor changed before write")
                    with (ROOT / name).open("r+b") as stream:
                        stream.write(body)
                        stream.truncate()
                    report["edits"].append(name)
                    save(OUT / "report.json", report)
                with InputBatch(ROOT) as after_batch:
                    after = {name: read(after_batch, name)[0] for name in plan["current_input_states"]}
                    for name, body in outputs.items():
                        state, actual = read(after_batch, name)
                        require(actual == body and all(state["metadata"][key] == original[name][0]["metadata"][key] for key in ("st_dev", "st_ino", "st_mode", "st_nlink", "st_uid", "st_gid")), "Owned inode changed")
                    require(read(after_batch, ".git/index") == (pre["runtime_index_state"], index), "Index changed")
                    admitted = capture_members(ROOT, member["source"])
                    function, = admitted["functions"]
                    require(admitted["target"] == plan["target"] and not admitted["data_sections"] and not any(admitted["section_placements"].values()) and function["selector"] == member["selector"] and function["symbol"] == member["symbol"] and function["file_start"] == member["original_file_start"] and function["original"] == (ROOT / member["original_path"]).read_bytes(), "Strict52B admission changed")
                    argv, _ = configured_command(ROOT, ROOT / member["source"], Path(member["configured_arguments"][-1]))
                    require(argv == member["configured_arguments"], "Profile changed")
                for name, expected in states.items():
                    if name not in outputs:
                        require(read(stable, name)[0] == expected, f"Stable/foreign input changed: {name}")
        report.update(status="applied reviewed SHOP52B source/layout outputs; POST pending", owned_post_sha256=plan["proposed_output_sha256"], before_states={n: original[n][0] for n in outputs}, after_states={n: after[n] for n in outputs}, runtime_index_state=pre["runtime_index_state"], pre_review=args.pre_review, pre_review_sha256=args.pre_review_sha256, foreign_report=args.foreign_report, foreign_report_sha256=args.foreign_report_sha256, foreign_states=foreign_states, guards_and_writer_closed=True)
        save(OUT / "report.json", report)
        plan["current_input_states"] = after
        plan["include_input_states"][member["source"]] = after[member["source"]]
        member.update(record=asdict(new_record), source_sha256=sha(outputs[member["source"]]))
        for unit in plan["normal_build_units"]:
            if unit["source"] == member["source"]:
                unit.update(records=[asdict(new_record)], source_sha256=member["source_sha256"])
        plan.update(application_report=str((OUT / "report.json").relative_to(ROOT)), application_sha256=sha((OUT / "report.json").read_bytes()), accepted_pre_sha256=args.pre_summary_sha256, foreign_states=foreign_states, runtime_index_state=pre["runtime_index_state"])
        save(POST_PLAN, plan)
    except BaseException as error:
        if OUT.exists():
            report.update(status="failed; keep applied outputs and fix cause", error=f"{type(error).__name__}: {error}")
            save(OUT / "report.json", report)
        raise


if __name__ == "__main__":
    main()
