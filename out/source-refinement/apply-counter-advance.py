"""Apply only the six reviewed COUNTER outputs after accepted native/CPP PRE."""
import argparse
from dataclasses import asdict
import hashlib
import json
from pathlib import Path
from harness.build.compiler import load_compiler_configuration, resolve_compiler_settings
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.domain.functions import parse_function_records

root = Path.cwd()
base = root / "out/source-refinement"
draft = base / "counter-advance-amended-draft"
plan_sha = "aa82f851029d93b85ec11ea8b3f51df2b68411cbf5a772d9df107b158b1493b5"
parser = argparse.ArgumentParser()
parser.add_argument("--pre-summary-sha256", required=True)
parser.add_argument("--pre-expansion-sha256", required=True)
parser.add_argument("--pre-index-sha256", required=True)
args = parser.parse_args()
sha = lambda value: hashlib.sha256(value).hexdigest()
def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")
def read(batch, name):
    return batch.read(root / name, max_bytes=64 * 1024 * 1024, include_metadata=True, single_link=True)

data = (draft / "plan.json").read_bytes()
assert sha(data) == plan_sha
plan = json.loads(data)
outputs = {name: (draft / "files" / name).read_bytes() for name in plan["proposed_output_sha256"]}
assert len(outputs) == 6
for name, content in outputs.items():
    assert sha(content) == plan["proposed_output_sha256"][name], name
out = base / "counter-advance-application"
assert not out.exists() and not (base / "counter-advance-post-plan.json").exists()
with acquire_writer(root):
    with InputBatch(root) as batch:
        before = {}
        def bind(name):
            state, content = read(batch, name)
            assert name not in before or before[name] == state, name
            before[name] = state
            return content
        assert sha(bind("out/source-refinement/counter-advance-amended-draft/plan.json")) == plan_sha
        bind(Path(__file__).resolve().relative_to(root).as_posix())
        native_data = bind("out/source-refinement/counter-advance-pre/summary.json")
        cpp_data = bind("out/source-refinement/counter-advance-expansions-pre.json")
        assert sha(native_data) == args.pre_summary_sha256 and sha(cpp_data) == args.pre_expansion_sha256
        native, cpp = json.loads(native_data), json.loads(cpp_data)
        assert native["status"] == "complete PRE captures" and native["member_count"] == 7 and native["complete_original_bytes"] == 280 and native["guards_and_writer_closed"]
        assert cpp["status"] == "configured emissions verified" and cpp["stage"] == "pre" and len(cpp["members"]) == 7 and cpp["guards_and_writer_closed"]
        expected_selectors = {m["selector"] for m in plan["selected"] + plan["existing_consumers"]}
        assert {m["selector"] for m in native["members"]} == {m["selector"] for m in cpp["members"]} == expected_selectors
        terminal = json.loads(bind("out/source-refinement/counter-advance-pre/terminal-input-states.json"))
        for expected_states in (plan["current_input_states"], terminal, cpp["current_input_states"]):
            for name, expected in expected_states.items():
                bind(name)
                assert before[name] == expected, name
        for name in outputs:
            assert sha(bind("out/source-refinement/counter-advance-amended-draft/files/" + name)) == plan["proposed_output_sha256"][name]
        index = bind(".git/index")
        index_state = before[".git/index"]
        assert sha(index) == args.pre_index_sha256
        pre = {name: (before[name], bind(name)) for name in outputs}
        assert all(state is not None and content is not None for state, content in pre.values())
    out.mkdir()
    (out / "index.pre.bin").write_bytes(index)
    for name, (state, content) in pre.items():
        saved = out / "pre" / name
        saved.parent.mkdir(parents=True, exist_ok=True)
        saved.write_bytes(content)
    with InputBatch(root) as stable:
        for name, expected in before.items():
            if name not in outputs:
                assert read(stable, name)[0] == expected, name
        with InputBatch(root) as owned:
            for name in outputs:
                assert read(owned, name) == pre[name], name
        for name, content in outputs.items():
            assert (root / name).read_bytes() == pre[name][1], name
            with (root / name).open("r+b") as stream:
                stream.write(content)
                stream.truncate()
        for name, expected in before.items():
            if name not in outputs:
                assert read(stable, name)[0] == expected, name
    with InputBatch(root) as batch:
        after = {name: read(batch, name)[0] for name in plan["current_input_states"]}
        for name, content in outputs.items():
            state, actual = read(batch, name)
            assert actual == content and state["metadata"]["st_ino"] == pre[name][0]["metadata"]["st_ino"], name
        assert read(batch, ".git/index") == (index_state, index)
        flags, compilers = load_compiler_configuration(root)
        for member in plan["selected"] + plan["existing_consumers"]:
            text = read(batch, member["source"])[1].decode()
            assert [asdict(r) for r in parse_function_records(text)] == [member["proposed_record"]]
            assert asdict(resolve_compiler_settings(member["source"], text, flags, compilers)) == member["effective_settings"]
report = {"status": "applied six reviewed outputs; POST pending", "plan_sha256": plan_sha, "accepted_pre_sha256": args.pre_summary_sha256, "accepted_cpp_pre_sha256": args.pre_expansion_sha256, "owned_outputs": plan["proposed_output_sha256"], "before_states": {n: pre[n][0] for n in outputs}, "after_states": {n: after[n] for n in outputs}, "index_before": index_state, "index_after": index_state, "guards_and_writer_closed": True}
save(out / "report.json", report)
plan["current_input_states"] = after
plan["application_report"] = "out/source-refinement/counter-advance-application/report.json"
plan["application_sha256"] = sha((out / "report.json").read_bytes())
plan["accepted_pre_sha256"] = args.pre_summary_sha256
plan["accepted_cpp_pre_sha256"] = args.pre_expansion_sha256
save(base / "counter-advance-post-plan.json", plan)
print("Applied six outputs; index preserved; POST remains pending")
