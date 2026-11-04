"""Check the selected eight Battle15 records and shared target gates after POST."""
import argparse
import hashlib
import json
import os
from pathlib import Path
from harness.analysis.index import connect
from harness.common.inputs import InputBatch
from harness.common.process import ProcessCleanupError, run_bounded
from harness.domain.functions import parse_function_records
from harness.match.extraction import read_function_bytes

root = Path(__file__).resolve().parents[2]
assert Path.cwd() == root
base = root / "out/source-refinement"
parser = argparse.ArgumentParser()
parser.add_argument("--post-summary-sha256", required=True)
parser.add_argument("--index-successor-sha256", required=True)
args = parser.parse_args()
sha = lambda body: hashlib.sha256(body).hexdigest()
PLAN_SHA = "7a21e5bd3892da26297653ffb8659a7d634b55d971844c90f1f27c0b50262c09"
APP_SHA = "f201be2aa4c38c2ed140411f44adb4243b7f16b283baac9b2d33d459cf4f1fd2"
PRE_SHA = "a917dddacd2e4d3579d75ca1071dac03bbe7fe9086fec0f661aae7b9d749833d"
plan_name = "out/source-refinement/battle15-01-metadata-post-plan.json"
data = (root / plan_name).read_bytes()
assert sha(data) == PLAN_SHA
plan = json.loads(data)
members = plan["members"]
assert len(members) == 8 and sum(m["original_size"] for m in members) == 740
out = base / "battle15-01-metadata-final-checks"
assert not out.exists()
out.mkdir()
environment = dict(os.environ)
environment["CMAKE_BUILD_PARALLEL_LEVEL"] = "1"
summary = {"status": "running", "commands": [], "limits": "Eight selected records only; retain actual global source validation and shared target gates without claiming whole-target strict metadata attachment."}
def save():
    (out / "summary.json").write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n")
def run(label, argv, allowed=(0,)):
    guard()
    print(label, flush=True)
    with (out / (label + ".stdout")).open("wb") as stdout, (out / (label + ".stderr")).open("wb") as stderr:
        result = run_bounded(root, argv, env=environment, output_limit=64 * 1024 * 1024, on_output=lambda stream, chunk: (stdout if stream == "stdout" else stderr).write(chunk))
    (out / (label + ".process.json")).write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    summary["commands"].append({"label": label, "argv": argv, "exit_code": result["exit_code"], "failure": result["failure"]})
    save()
    assert not result["failure"] and result["exit_code"] in allowed, label + " failed; retain applied files and outputs"
    guard()
    return result
try:
    with InputBatch(root) as inputs:
        states = {}
        def bind(name, digest=None):
            state, body = inputs.read(root / name, max_bytes=64 * 1024 * 1024, include_metadata=True, single_link=True)
            assert name not in states or states[name] == state, name
            assert digest is None or state is not None and state["sha256"] == digest, name
            states[name] = state
            return body
        def guard():
            for name in list(states):
                bind(name)
        for name, expected in plan["current_input_states"].items():
            bind(name)
            assert states[name] == expected, name
        bind(plan_name, PLAN_SHA)
        bind(Path(__file__).resolve().relative_to(root).as_posix())
        app = json.loads(bind(plan["application_report"], APP_SHA))
        bind(".git/index")
        assert app["index_before"] == app["index_after"] and app["guards_and_writer_closed"]
        successor_name = "out/source-refinement/battle15-01-index-successor.json"
        successor = json.loads(bind(successor_name, args.index_successor_sha256))
        assert states[".git/index"] == successor["accepted_current_state"] and states[".git/index"]["sha256"] == successor["accepted_current_index_sha256"]
        assert successor["historical_index_sha256"] == app["index_after"]["sha256"]
        assert successor["exact_logical_index_delta"] == ["tools/rust/bof3-audio/driver/README.md", "tools/rust/bof3-audio/driver/audio/xa.s"]
        assert all(successor[key] is True for key in ("all_other_index_entries_unchanged", "current_index_equals_successor_tree", "current_staged_diff_empty", "old_index_equals_parent_tree"))
        summary["index_successor"] = {"report": successor_name, "sha256": args.index_successor_sha256, "accepted_current_state": successor["accepted_current_state"]}
        pre_root, post_root = "out/source-refinement/battle15-01-metadata-pre", "out/source-refinement/battle15-01-metadata-post"
        pre = json.loads(bind(pre_root + "/summary.json", PRE_SHA))
        post = json.loads(bind(post_root + "/summary.json", args.post_summary_sha256))
        assert pre["status"] == "complete PRE captures" and post["status"] == "complete POST captures"
        assert post["runtime_index_state"] == app["index_before"] == app["index_after"]
        assert all(s["member_count"] == 8 and s["complete_original_bytes"] == 740 and s["guards_and_writer_closed"] for s in (pre, post))
        assert sum(r["report"]["byte_match"] for r in pre["members"]) == 7 and sum(r["report"]["original_size"] for r in pre["members"] if r["report"]["byte_match"]) == 448
        partial = next(r["report"] for r in pre["members"] if not r["report"]["byte_match"])
        assert partial["function"] == "func_8009B03C" and partial["current_size"] == partial["original_size"] == 292 and partial["instruction_count"] == {"original": 73, "current": 73, "matching": 54, "match_percent": 73.97} and partial["first_mismatch"]["original_offset"] == partial["first_mismatch"]["current_offset"] == 0
        pre_rows = {m["selector"]: m for m in pre["members"]}
        post_rows = {m["selector"]: m for m in post["members"]}
        assert set(pre_rows) == set(post_rows) == {m["selector"] for m in members}
        for member in members:
            symbol, selector = member["symbol"], member["selector"]
            before, after = pre_rows[selector], post_rows[selector]
            assert after["complete_pre_images_and_metrics_equal"] and before["actual_comparison_exit_code"] == after["actual_comparison_exit_code"]
            for key in ("instruction_count", "first_mismatch", "original_size", "current_size", "size_delta", "byte_match", "exact_match", "function_image"):
                assert before["report"][key] == after["report"][key], (selector, key)
            for suffix, field in (("original.bin", "original_sha256"), ("current.bin", "current_sha256")):
                first = bind(pre_root + f"/{symbol}.{suffix}")
                second = bind(post_root + f"/{symbol}.{suffix}")
                assert first == second and len(first) == member["original_size"] and sha(first) == after["report"]["function_image"][field]
            for folder, row in ((pre_root, before), (post_root, after)):
                elf = bind(folder + f"/{symbol}.linked.o", row["elf_sha256"])
                assert read_function_bytes(elf, function_name=symbol, address=member["boundary"]["virtual_start"]).content == bind(folder + f"/{symbol}.current.bin")
            assert len(parse_function_records(bind(member["source"]).decode())) == 1
        target = plan["target"]
        status = json.loads(run("rz-status", ["bin/harness", "analysis", "rz-project", "status", target, "--json"], (0, 1))["stdout"])
        assert status["target"] == target
        if not status["fresh"]:
            run("rz-analysis", ["bin/harness", "analysis", "rz-project", "analyze", target])
        try:
            connection = connect(root)
        except (FileNotFoundError, ValueError):
            run("index", ["bin/harness", "analysis", "index"])
        else:
            connection.close()
        result = run("selected-metadata", ["bin/harness", "source", "metadata", "check", target, *sorted(pre_rows), "--json"])
        report = json.loads(result["stdout"])
        assert report["passed"] and not report["incomplete"] and not report["findings"] and {r["selector"] for r in report["records"]} == set(pre_rows)
        for row in report["records"]:
            actual, expected = row["measurement"], pre_rows[row["selector"]]["report"]
            assert not row["findings"] and actual["state"] == "live" and actual["status"] == ("exact" if expected["byte_match"] else "partial") and actual["instruction_count"] == expected["instruction_count"] and actual["match_percent"] == expected["instruction_count"]["match_percent"] and actual["original_size"] == expected["original_size"] and actual["current_size"] == expected["current_size"]
        run("symbols", ["bin/harness", "source", "symbols", "check", target])
        run("splat", ["bin/harness", "source", "splat", target])
        run("build", ["bin/harness", "build", target])
        result = run("source-validation", ["bin/harness", "source", "validate", "--out", str(out.relative_to(root) / "source-validation.json")], (0, 2))
        validation = json.loads((out / "source-validation.json").read_bytes())
        owner = next(r for r in validation["targets"] if r["target"] == target)
        assert validation["coverage_error"] is None and owner["coverage_error"] is None
        for member in members:
            rows = [r for r in owner["functions"] if r["source"] == member["source"] and int(r["address"], 16) == member["boundary"]["virtual_start"]]
            assert len(rows) == 1 and rows[0]["status"] != "invalid", member["selector"]
        summary["actual_source_validation"] = {"exit_code": result["exit_code"], "global_lifts": validation["lifts"], "owning_target_lifts": owner["lifts"], "selected_noninvalid": 8, "strict_whole_target_metadata_attachment_claimed": False}
        save()
        assert result["exit_code"] == 0 and validation["lifts"]["invalid"] == 0, "Global source validation failed; no baseline exemption"
        guard()
    summary.update(status="selected metadata and target gates/build complete; independent acceptance pending", selected_records=8, exact_records=7, exact_bytes=448, preserved_partial_records=1, preserved_partial_bytes=292, guards_and_index_closed=True)
    save()
except BaseException as error:
    summary.update(status="failed; keep applied files and fix cause", error=f"{type(error).__name__}: {error}")
    if isinstance(error, ProcessCleanupError):
        summary["cleanup"] = "unconfirmed; stop native work"
        summary["cleanup_observation"] = error.observed
    save()
    raise
