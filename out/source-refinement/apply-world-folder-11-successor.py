"""Apply the reviewed 12 world-folder-11 moves and 8 config edits after accepted PRE."""
import argparse
from dataclasses import asdict
import hashlib
import json
import os
from pathlib import Path
from harness.build.compilation import configured_command
from harness.build.compiler import load_compiler_configuration, resolve_compiler_settings
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.rename import rename_noreplace
from harness.domain.functions import parse_function_records
from harness.match.members import capture_members

root = Path.cwd()
assert root == Path(__file__).resolve().parents[2], "Run from repository root"
base = root / "out/source-refinement"
draft = base / "world-folder-11-ready-draft"
plan_name = "out/source-refinement/world-folder-11-ready-successor/plan.json"
plan_sha = "05f4fe165764b369285b7ae27c28ee3105cdc0b7abbec2d510376408979ea1c2"
parser = argparse.ArgumentParser()
parser.add_argument("--pre-summary-sha256", required=True)
parser.add_argument("--pre-index-sha256", required=True)
parser.add_argument("--pre-root", default="out/source-refinement/world-folder-11-pre")
args = parser.parse_args()
assert not Path(args.pre_root).is_absolute() and ".." not in Path(args.pre_root).parts
pre_root = root / args.pre_root
sha = lambda value: hashlib.sha256(value).hexdigest()
def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")
def read(batch, name):
    return batch.read(root / name, max_bytes=64 * 1024 * 1024, include_metadata=True, single_link=True)
data = (root / plan_name).read_bytes()
assert sha(data) == plan_sha
plan = json.loads(data)
members = plan["selected"]
outputs = {n: (draft / "proposed" / n).read_bytes() for n in plan["proposed_output_sha256"]}
config_outputs = {n: b for n, b in outputs.items() if n.startswith("config/")}
owned = set(plan["owned_paths"]["final"])
assert len(members) == 12 and len(outputs) == 20 and len(config_outputs) == 8 and len(owned) == 32
for n, b in outputs.items():
    assert sha(b) == plan["proposed_output_sha256"][n], n
for m in members:
    assert outputs[m["destination"]] == (draft / "owned-pre" / m["source"]).read_bytes()
out = base / "world-folder-11-application"
assert not out.exists() and not (base / "world-folder-11-post-plan.json").exists()
report = {"status": "running", "plan_sha256": plan_sha, "accepted_pre_sha256": args.pre_summary_sha256, "moves": [], "config_edits": []}
try:
    with acquire_writer(root):
        with InputBatch(root) as batch:
            before = {}
            def bind(name):
                state, body = read(batch, name)
                assert name not in before or before[name] == state, name
                before[name] = state
                return body
            assert sha(bind(plan_name)) == plan_sha
            bind(Path(__file__).resolve().relative_to(root).as_posix())
            pre_data = bind(args.pre_root + "/summary.json")
            assert sha(pre_data) == args.pre_summary_sha256
            pre_summary = json.loads(pre_data)
            assert pre_summary["plan_sha256"] == plan_sha
            assert pre_summary["status"] == "complete PRE captures" and pre_summary["member_count"] == 12 and pre_summary["complete_original_bytes"] == 424 and pre_summary["guards_and_writer_closed"]
            expected_selectors = {m["selector"] for m in members} | {m["selector"] for g in plan["untouched_authored_siblings"] for m in g["members"]}
            assert len(expected_selectors) == 12 and {m["selector"] for m in pre_summary["members"]} == expected_selectors
            terminal = json.loads(bind(args.pre_root + "/terminal-input-states.json"))
            for expected_states in (plan["current_input_states"], terminal):
                for n, expected in expected_states.items():
                    bind(n)
                    assert before[n] == expected, n
            for n in outputs:
                assert sha(bind("out/source-refinement/world-folder-11-ready-draft/proposed/" + n)) == plan["proposed_output_sha256"][n]
            index = bind(".git/index")
            index_state = before[".git/index"]
            assert sha(index) == args.pre_index_sha256 and index_state == pre_summary["runtime_index_state"]
            pre = {n: (before[n], bind(n)) for n in plan["owned_paths"]["existing"]}
            assert len(pre) == 20 and all(state is not None and body is not None for state, body in pre.values())
            assert all(before[n] is None for n in plan["owned_paths"]["absent_destinations"])
        out.mkdir()
        (out / "index.pre.bin").write_bytes(index)
        save(out / "report.json", report)
        for n, (state, body) in pre.items():
            saved = out / "pre" / n
            saved.parent.mkdir(parents=True, exist_ok=True)
            saved.write_bytes(body)
        with InputBatch(root) as stable:
            for n, expected in before.items():
                if n not in owned:
                    assert read(stable, n)[0] == expected, n
            with InputBatch(root) as mutable:
                for n in owned:
                    assert read(mutable, n) == (pre[n] if n in pre else (None, None)), n
            directory = os.open(root, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW | os.O_CLOEXEC)
            try:
                for m in members:
                    assert (root / m["source"]).read_bytes() == pre[m["source"]][1]
                    rename_noreplace(m["source"], m["destination"], src_dir_fd=directory, dst_dir_fd=directory)
                    report["moves"].append({"source": m["source"], "destination": m["destination"]})
                    save(out / "report.json", report)
            finally:
                os.close(directory)
            for n, body in config_outputs.items():
                assert (root / n).read_bytes() == pre[n][1], n
                with (root / n).open("r+b") as stream:
                    stream.write(body)
                    stream.truncate()
                report["config_edits"].append(n)
                save(out / "report.json", report)
            for n, expected in before.items():
                if n not in owned:
                    assert read(stable, n)[0] == expected, n
        with InputBatch(root) as batch:
            after = {n: read(batch, n)[0] for n in plan["current_input_states"]}
            for m in members:
                old, new = m["source"], m["destination"]
                state, body = read(batch, new)
                assert read(batch, old) == (None, None) and body == pre[old][1]
                assert state["sha256"] == pre[old][0]["sha256"]
                assert all(state["metadata"][k] == pre[old][0]["metadata"][k] for k in ("st_dev", "st_ino", "st_mode", "st_nlink", "st_uid", "st_gid", "st_size", "st_mtime_ns"))
            for n, body in config_outputs.items():
                state, actual = read(batch, n)
                assert actual == body and state["metadata"]["st_ino"] == pre[n][0]["metadata"]["st_ino"], n
            assert read(batch, ".git/index") == (index_state, index)
            flags, compilers = load_compiler_configuration(root)
            for m in members:
                text = read(batch, m["destination"])[1].decode()
                assert [asdict(rec) for rec in parse_function_records(text)] == [m["current_record"]]
                assert asdict(resolve_compiler_settings(m["destination"], text, flags, compilers)) == m["effective_settings"]
                argv, _ = configured_command(root, root / m["destination"], Path(m["configured_arguments"][-1]), allow_grouped=True)
                assert argv == m["proposed_configured_arguments"]
                captured = capture_members(root, m["destination"])
                assert captured["target"] == m["target"] and len(captured["functions"]) == 1 and not captured["data_sections"] and not any(captured["section_placements"].values())
                f = captured["functions"][0]
                assert f["selector"] == m["selector"] and f["symbol"] == m["symbol"] and f["file_start"] == m["file_start"] and sha(f["original"]) == m["original_sha256"] and len(f["original"]) == m["original_size"]
    report.update(status="applied 12 inode-preserving moves and 8 config edits; POST pending", owned_post_sha256=plan["proposed_output_sha256"], before_states={n: pre[n][0] for n in pre}, after_states={n: after[n] for n in owned}, index_before=index_state, index_after=index_state, guards_and_writer_closed=True)
    save(out / "report.json", report)
    plan["current_input_states"] = after
    plan["application_report"] = "out/source-refinement/world-folder-11-application/report.json"
    plan["application_sha256"] = sha((out / "report.json").read_bytes())
    plan["accepted_pre_sha256"] = args.pre_summary_sha256
    plan["accepted_pre_root"] = args.pre_root
    for m in members:
        m["old_source"], m["source"] = m["source"], m["destination"]
        m["configured_arguments"] = m["proposed_configured_arguments"]
    for unit in plan["normal_build_units"]:
        moved = next((m for m in members if m["old_source"] == unit["source"]), None)
        if moved is not None:
            unit["source"] = moved["destination"]
            unit["configured_arguments"] = moved["proposed_configured_arguments"]
    save(base / "world-folder-11-post-plan.json", plan)
    print("Applied 12 moves and 8 config edits; raw index unchanged; POST pending")
except BaseException as error:
    if out.exists():
        report.update(status="failed; retain applied files and repair cause", error=f"{type(error).__name__}: {error}")
        save(out / "report.json", report)
    raise
