"""Finish Battle15 build/validation after the retained generated-Splat guard refusal."""
import hashlib
import json
import os
import re
from pathlib import Path
from harness.build.compilation import configured_command
from harness.common.inputs import InputBatch
from harness.common.process import ProcessCleanupError, run_bounded
from harness.domain.functions import parse_function_records
from harness.match.extraction import read_function_bytes

root = Path(__file__).resolve().parents[2]
assert Path.cwd() == root
base = root / "out/source-refinement"
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
out = base / "battle15-01-metadata-remaining-checks"
assert not out.exists()
out.mkdir()
environment = dict(os.environ)
environment["CMAKE_BUILD_PARALLEL_LEVEL"] = "1"
summary = {"status": "running", "commands": [], "limits": "Resume only normal build/global validation. Continuous full-state guards cover188 unmodified plan inputs and accepted evidence/index. Exactly8generated Splat ASMs are verified in separate closed phases before/after build against approved hashes and complete original words; mtime/ctime may refresh only there. Retain prior live checks without rerunning. No whole-target strict attachment claim."}
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
generated = {m["original_assembly_path"] for m in members}
assert len(generated) == 8
historical = "out/source-refinement/battle15-01-metadata-final-checks"
EVIDENCE = {'out/source-refinement/battle15-01-metadata-final-checks/summary.json': '74055b644a07c57a4d2db473d4190978a49f9efe397cae59ff9783e531c77d32', 'out/source-refinement/battle15-01-post-splat-state-observation.json': 'f6853d60dc8b0511bc92e945dbff6aecc0c4164f6a4e86839c48bd9bc210770d', 'out/source-refinement/check-battle15-01-final.py': 'cc5bd9844060abc23709e3a846fd89570d3b94cafe45e0b588db358892bc30ae', 'out/source-refinement/battle15-01-metadata-final-checks/rz-status.process.json': 'e8c8b4361e5348e6648ed752f90bf758df7b2111340d367bdd82cabceb37805e', 'out/source-refinement/battle15-01-metadata-final-checks/index.process.json': 'e1122242a3fa8d374dbf4fbf41c627b2bf7f9a8dd8d2d8d92906ffc074c9bc31', 'out/source-refinement/battle15-01-metadata-final-checks/selected-metadata.process.json': 'ac924ea9ebc78185bbf153e6db28e3fed02497bd16fc79fc5e47bed3afa893e8', 'out/source-refinement/battle15-01-metadata-final-checks/symbols.process.json': '7a8d3e7855641143cbd5c8cd97c4cc5b6934cbad839df3ec9954f98683ba4010', 'out/source-refinement/battle15-01-metadata-final-checks/splat.process.json': 'a4fe0a518d2ec41539ab1dd7c9eca63f782553f00fa4b33dcca5815c4466de0d'}

def assembly_phase(label):
    result = {}
    with InputBatch(root) as phase:
        for member in members:
            name = member["original_assembly_path"]
            state, body = phase.read(root / name, max_bytes=64 * 1024 * 1024, include_metadata=True, single_link=True)
            expected = plan["current_input_states"][name]
            assert state["sha256"] == expected["sha256"] == member["original_assembly_sha256"], name
            assert {k:v for k,v in state["metadata"].items() if k not in ("st_mtime_ns", "st_ctime_ns")} == {k:v for k,v in expected["metadata"].items() if k not in ("st_mtime_ns", "st_ctime_ns")}, name
            raw = bind(member["original_path"])
            binary = bind(plan["original_binary"])
            boundary = member["boundary"]
            assert raw == binary[boundary["file_start"]:boundary["file_end"]] and sha(raw) == member["original_sha256"] and len(raw) == member["original_size"]
            words = re.findall(rb'/\*\s*([0-9A-Fa-f]+)\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})\s*\*/', body)
            assert len(words) == len(member["original_instructions"]) and len(words) * 4 == len(raw)
            for i, ((offset, address, encoded), instruction) in enumerate(zip(words, member["original_instructions"])):
                assert int(offset, 16) == instruction["file_offset"] == boundary["file_start"] + 4*i
                assert int(address, 16) == instruction["address"] == boundary["virtual_start"] + 4*i
                assert bytes.fromhex(encoded.decode()) == bytes.fromhex(instruction["bytes"]) == raw[4*i:4*i+4]
            result[name] = state
    (out / (label + "-generated-asm-states.json")).write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    summary[label + "_generated_asm_phase_closed"] = True
    save()

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
            if name in generated:
                continue
            bind(name)
            assert states[name] == expected, name
        assert len(states) == 188 and generated.isdisjoint(states)
        for name, digest in EVIDENCE.items():
            bind(name, digest)
        failed = json.loads(bind(historical + "/summary.json"))
        assert failed["status"] == "failed; keep applied files and fix cause" and "func_80096AB0.s" in failed["error"] and "watched input entry or ancestor changed" in failed["error"]
        assert [c["label"] for c in failed["commands"]] == ["rz-status", "index", "selected-metadata", "symbols", "splat"]
        for command in failed["commands"]:
            label = command["label"]
            process = json.loads(bind(historical + f"/{label}.process.json"))
            assert process["argv"] == command["argv"] and process["exit_code"] == command["exit_code"] == 0 and process["failure"] is command["failure"] is None
            for stream in ("stdout", "stderr"):
                assert bind(historical + f"/{label}.{stream}").decode() == process[stream]
        observation = json.loads(bind("out/source-refinement/battle15-01-post-splat-state-observation.json"))
        assert {r["path"] for r in observation["observed_plan_drifts"]} == generated
        for row in observation["observed_plan_drifts"]:
            assert row["expected"] == plan["current_input_states"][row["path"]] and row["hash_equal"] and set(row["metadata_delta"]) == {"st_mtime_ns", "st_ctime_ns"}
        summary["retained_completed_commands"] = failed["commands"]
        bind(plan_name, PLAN_SHA)
        bind(Path(__file__).resolve().relative_to(root).as_posix())
        app = json.loads(bind(plan["application_report"], APP_SHA))
        bind(".git/index")
        assert app["index_before"] == app["index_after"] and app["guards_and_writer_closed"]
        successor_name = "out/source-refinement/battle15-01-index-successor.json"
        successor = json.loads(bind(successor_name, "0c3136bca13b37f456aed5ad6254702bddf283f781810b70df0461a17c0a5863"))
        assert states[".git/index"] == successor["accepted_current_state"] and states[".git/index"]["sha256"] == successor["accepted_current_index_sha256"]
        assert successor["historical_index_sha256"] == app["index_after"]["sha256"]
        assert successor["exact_logical_index_delta"] == ["tools/rust/bof3-audio/driver/README.md", "tools/rust/bof3-audio/driver/audio/xa.s"]
        assert all(successor[key] is True for key in ("all_other_index_entries_unchanged", "current_index_equals_successor_tree", "current_staged_diff_empty", "old_index_equals_parent_tree"))
        summary["index_successor"] = {"report": successor_name, "sha256": "0c3136bca13b37f456aed5ad6254702bddf283f781810b70df0461a17c0a5863", "accepted_current_state": successor["accepted_current_state"]}
        pre_root, post_root = "out/source-refinement/battle15-01-metadata-pre", "out/source-refinement/battle15-01-metadata-post"
        pre = json.loads(bind(pre_root + "/summary.json", PRE_SHA))
        post = json.loads(bind(post_root + "/summary.json", "55e0cbe71a950de1fbce214816d5f9b1387156d1c063abf22be7a643f24cd5c8"))
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
            arguments, _ = configured_command(root, root / member["source"], Path(member["configured_arguments"][-1]))
            assert arguments == member["configured_arguments"]
        target = plan["target"]
        report = json.loads(json.loads(bind(historical + "/selected-metadata.process.json"))["stdout"])
        assert report["passed"] and not report["incomplete"] and not report["findings"] and {r["selector"] for r in report["records"]} == set(pre_rows)
        for row in report["records"]:
            actual, expected = row["measurement"], pre_rows[row["selector"]]["report"]
            assert not row["findings"] and actual["state"] == "live" and actual["status"] == ("exact" if expected["byte_match"] else "partial") and actual["instruction_count"] == expected["instruction_count"] and actual["match_percent"] == expected["instruction_count"]["match_percent"] and actual["original_size"] == expected["original_size"] and actual["current_size"] == expected["current_size"]
        assembly_phase("pre-build")
        guard()
        run("build", ["bin/harness", "build", target])
        assembly_phase("post-build")
        guard()
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
    summary.update(status="remaining build/global validation complete; retained live checks unchanged; independent acceptance pending", selected_records=8, exact_records=7, exact_bytes=448, preserved_partial_records=1, preserved_partial_bytes=292, guards_and_index_closed=True)
    save()
except BaseException as error:
    summary.update(status="failed; keep applied files and fix cause", error=f"{type(error).__name__}: {error}")
    if isinstance(error, ProcessCleanupError):
        summary["cleanup"] = "unconfirmed; stop native work"
        summary["cleanup_observation"] = error.observed
    save()
    raise
