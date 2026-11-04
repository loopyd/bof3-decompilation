"""Apply only the eight reviewed Battle15 metadata proposals after accepted PRE."""
from dataclasses import asdict
import hashlib
import json
from pathlib import Path
from harness.build.compilation import configured_command
from harness.build.compiler import load_compiler_configuration, resolve_compiler_settings
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.lexicon import iter_c_lexemes
from harness.domain.functions import parse_function_records
from harness.domain.tags import parse_progress_tags
from harness.match.members import capture_members

root = Path(__file__).resolve().parents[2]
assert Path.cwd() == root
base = root / "out/source-refinement"
draft = base / "metadata-battle15-01-ready-draft"
PLAN_SHA = "73f934a7a5688d2272bd9f0094edf1f65f11f0651d600878b5971835cae4227d"
PRE_SHA = "a917dddacd2e4d3579d75ca1071dac03bbe7fe9086fec0f661aae7b9d749833d"
sha = lambda body: hashlib.sha256(body).hexdigest()
plan_data = (draft / "plan.json").read_bytes()
assert sha(plan_data) == PLAN_SHA
plan = json.loads(plan_data)
outputs = {name: (draft / "files" / name).read_bytes() for name in plan["proposed_output_sha256"]}
assert len(outputs) == 8 and len(plan["current_input_states"]) == 196
assert sum(m["original_size"] for m in plan["members"]) == 740
out = base / "battle15-01-metadata-application"
assert not out.exists() and not (base / "battle15-01-metadata-post-plan.json").exists()
report = {"status": "running", "parent_plan_sha256": PLAN_SHA, "accepted_pre_sha256": PRE_SHA, "changed": []}
def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")
def read(batch, name):
    return batch.read(root / name, max_bytes=64 * 1024 * 1024, include_metadata=True, single_link=True)
def code(text):
    for match in reversed(list(iter_c_lexemes(text))):
        if match.group().startswith(("/*", "//")):
            text = text[:match.start()] + text[match.end():]
    return "".join(text.split())
try:
    with acquire_writer(root):
        with InputBatch(root) as batch:
            before = {}
            def bind(name):
                state, body = read(batch, name)
                assert name not in before or before[name] == state, name
                before[name] = state
                return body
            assert sha(bind("out/source-refinement/metadata-battle15-01-ready-draft/plan.json")) == PLAN_SHA
            bind(Path(__file__).resolve().relative_to(root).as_posix())
            pre_data = bind("out/source-refinement/battle15-01-metadata-pre/summary.json")
            assert sha(pre_data) == PRE_SHA
            pre_summary = json.loads(pre_data)
            assert pre_summary["status"] == "complete PRE captures" and pre_summary["guards_and_writer_closed"] and pre_summary["member_count"] == 8 and pre_summary["complete_original_bytes"] == 740 and pre_summary["plan_sha256"] == PLAN_SHA
            assert {m["selector"] for m in pre_summary["members"]} == {m["selector"] for m in plan["members"]}
            terminal_data = bind("out/source-refinement/battle15-01-metadata-pre/terminal-input-states.json")
            assert sha(terminal_data) == '9d405a1f1ba9047c89af817be9844f66db7977040f748c4d714e46578147aea5'
            terminal = json.loads(terminal_data)
            for expected_states in (plan["current_input_states"], terminal):
                for name, expected in expected_states.items():
                    bind(name)
                    assert before[name] == expected, name
            index = bind(".git/index")
            index_state = before[".git/index"]
            pre = {}
            flags, compilers = load_compiler_configuration(root)
            for member in plan["members"]:
                name = member["source"]
                old = bind(name)
                proposed = bind("out/source-refinement/metadata-battle15-01-ready-draft/files/" + name)
                assert proposed == outputs[name] and sha(old) == member["current_sha256"] and sha(proposed) == member["proposed_sha256"] == plan["proposed_output_sha256"][name]
                assert old == bind("out/source-refinement/metadata-battle15-01-ready-draft/owned-pre/" + name)
                assert code(old.decode()) == code(proposed.decode()) and parse_progress_tags(old.decode()) == parse_progress_tags(proposed.decode()) and member["progress_literal"] in proposed.decode()
                records = parse_function_records(proposed.decode())
                assert [asdict(r) for r in records] == [member["proposed_record"]]
                assert asdict(resolve_compiler_settings(name, old.decode(), flags, compilers)) == asdict(resolve_compiler_settings(name, proposed.decode(), flags, compilers)) == member["effective_settings"]
                argv, _ = configured_command(root, root / name, Path(member["configured_arguments"][-1]), allow_grouped=True)
                assert argv == member["configured_arguments"]
                pre[name] = (before[name], old)
        out.mkdir()
        (out / "index.pre.bin").write_bytes(index)
        save(out / "report.json", report)
        for name, (_, body) in pre.items():
            saved = out / "pre" / name
            saved.parent.mkdir(parents=True, exist_ok=True)
            saved.write_bytes(body)
        with InputBatch(root) as stable:
            for name, expected in before.items():
                if name not in outputs:
                    assert read(stable, name)[0] == expected, name
            for name, body in outputs.items():
                assert (root / name).read_bytes() == pre[name][1], name
                with (root / name).open("r+b") as stream:
                    stream.write(body)
                    stream.truncate()
                report["changed"].append(name)
                save(out / "report.json", report)
            for name, expected in before.items():
                if name not in outputs:
                    assert read(stable, name)[0] == expected, name
        with InputBatch(root) as batch:
            after = {name: read(batch, name)[0] for name in plan["current_input_states"]}
            for member in plan["members"]:
                name = member["source"]
                state, body = read(batch, name)
                assert body == outputs[name] and sha(body) == member["proposed_sha256"]
                assert all(state["metadata"][key] == pre[name][0]["metadata"][key] for key in ("st_dev", "st_ino", "st_mode", "st_nlink", "st_uid", "st_gid"))
                strict = capture_members(root, name)
                assert strict["target"] == plan["target"] and len(strict["functions"]) == 1 and not strict["data_sections"] and not any(strict["section_placements"].values())
                function = strict["functions"][0]
                assert function["selector"] == member["selector"] and function["symbol"] == member["symbol"] and function["file_start"] == member["boundary"]["file_start"] and len(function["original"]) == member["original_size"] and sha(function["original"]) == member["original_sha256"]
            assert read(batch, ".git/index") == (index_state, index)
    report.update(status="applied eight reviewed metadata proposals; POST and final checks pending", owned_outputs=plan["proposed_output_sha256"], before_states={name: state for name, (state, _) in pre.items()}, after_states={name: after[name] for name in outputs}, index_before=index_state, index_after=index_state, guards_and_writer_closed=True)
    save(out / "report.json", report)
    plan["current_input_states"] = after
    plan["application_report"] = "out/source-refinement/battle15-01-metadata-application/report.json"
    plan["application_sha256"] = sha((out / "report.json").read_bytes())
    plan["accepted_pre_sha256"] = PRE_SHA
    save(base / "battle15-01-metadata-post-plan.json", plan)
    print("Applied eight metadata proposals; bodies/profiles/progress/inodes/index preserved; POST pending")
except BaseException as error:
    if out.exists():
        report.update(status="failed; keep applied files and fix cause", error=f"{type(error).__name__}: {error}")
        save(out / "report.json", report)
    raise
