"""Finish folder10 validation and discovery after a foreign-source index change."""
import argparse
import hashlib
import json
import os
from pathlib import Path
from harness.common.inputs import InputBatch
from harness.common.process import ProcessCleanupError, run_bounded

root = Path(__file__).resolve().parents[2]
assert Path.cwd() == root
base = root / "out/source-refinement"
parser = argparse.ArgumentParser()
for option in ("post-plan-sha256", "post-summary-sha256", "application-sha256", "pre-index-sha256"):
    parser.add_argument("--" + option, required=True)
args = parser.parse_args()
sha = lambda body: hashlib.sha256(body).hexdigest()
plan_name = "out/source-refinement/world-folder-10-post-plan.json"
data = (root / plan_name).read_bytes()
assert sha(data) == args.post_plan_sha256
plan = json.loads(data)
members = plan["selected"] + [dict(m, source=g["source"], target=g["target"]) for g in plan["untouched_authored_siblings"] for m in g["members"]]
targets = sorted({m["target"] for m in members})
assert len(targets) == 11 and len(members) == 11 and sum(m["original_size"] for m in members) == 340
assert len(plan["selected"]) == 11 and len(plan["normal_build_units"]) == 22
sizes = {m["selector"]: m["original_size"] for m in members}
out = base / "world-folder-10-final-repair"
assert not out.exists()
out.mkdir()
environment = dict(os.environ)
environment["CMAKE_BUILD_PARALLEL_LEVEL"] = "1"
summary = {"status": "running", "commands": [], "limits": "Finite folder10 gates only; preserve all failed outputs and keep applied sources. Independent acceptance remains pending."}
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
        def bind(name):
            state, body = inputs.read(root / name, max_bytes=64 * 1024 * 1024, include_metadata=True, single_link=True)
            assert name not in states or state == states[name], name
            states[name] = state
            return body
        assert not any(name.startswith("out/splat/") for name in plan["current_input_states"]), "Generated Splat outputs must not be continuous authored inputs"
        for name, expected in plan["current_input_states"].items():
            bind(name)
            assert states[name] == expected, name
        assert sha(bind(plan_name)) == args.post_plan_sha256
        bind(Path(__file__).resolve().relative_to(root).as_posix())
        application_data = bind(plan["application_report"])
        assert sha(application_data) == args.application_sha256 == plan["application_sha256"]
        application = json.loads(application_data)
        assert application["guards_and_writer_closed"] and len(application["moves"]) == 11 and len(application["config_edits"]) == 11
        post_data = bind("out/source-refinement/world-folder-10-post/summary.json")
        assert sha(post_data) == args.post_summary_sha256
        post = json.loads(post_data)
        assert post["status"] == "complete POST captures" and post["member_count"] == 11 and post["complete_original_bytes"] == 340 and post["guards_and_writer_closed"]
        assert {m["selector"] for m in post["members"]} == set(sizes)
        index = bind(".git/index")
        assert sha(index) == args.pre_index_sha256 and states[".git/index"] == application["index_before"] == application["index_after"] == post["runtime_index_state"]
        (out / "index.pre.bin").write_bytes(index)
        def guard():
            for name in list(states):
                bind(name)
        guard()
        previous = base / "world-folder-10-final-checks"
        previous_summary = json.loads(bind(str((previous / "summary.json").relative_to(root))))
        assert previous_summary["status"] == "failed" and previous_summary["error"] == "AssertionError: "
        commands = previous_summary["commands"]
        for target in targets:
            for suffix in ("combiner", "symbols", "splat", "build"):
                label = target.replace("/", "-") + "-" + suffix
                row = next(c for c in commands if c["label"] == label)
                assert row["exit_code"] == 0 and row["failure"] is None
                for ext in ("process.json", "stdout", "stderr"):
                    bind(str((previous / (label + "." + ext)).relative_to(root)))
        baseline = json.loads(bind("out/source-refinement/world-folder-10-post-foreign-baseline.json"))
        for name, expected in baseline["foreign_tracked_states"].items():
            if name != "src/bof3/battle/func_801DAAE4.c":
                bind(name)
                assert states[name] == expected, name
        directory_fields = ("st_dev", "st_gid", "st_ino", "st_mode", "st_nlink", "st_uid")
        def directory_identities():
            return {name: {key: getattr((root / name).stat(follow_symlinks=False), key) for key in directory_fields} for name in baseline["foreign_directory_identities"]}
        assert directory_identities() == baseline["foreign_directory_identities"]
        def battle_endpoint():
            with InputBatch(root) as observation:
                state, _ = observation.read(root / "src/bof3/battle/func_801DAAE4.c", include_metadata=True, single_link=True)
                return state
        summary["foreign_battle_before"] = battle_endpoint()
        summary["predecessor"] = {"directory": str(previous.relative_to(root)), "successful_native_concern_groups": 11, "successful_normal_builds": 11, "preserved_failure": "stale reverse index source fingerprints for emi/battle/battle/03; run just index"}
        run("index-refresh", ["bin/harness", "analysis", "index"])
        run("source-validation", ["bin/harness", "source", "validate", "--out", str(out.relative_to(root) / "source-validation.json")])
        validation = json.loads((out / "source-validation.json").read_bytes())
        assert validation["coverage_error"] is None
        by_target = {r["target"]: r for r in validation["targets"]}
        for target in targets:
            assert by_target[target]["coverage_error"] is None and by_target[target]["lifts"]["invalid"] == 0, target
        patterns = out / "obsolete-source-paths.txt"
        patterns.write_text("".join(m["old_source"] + "\n" for m in plan["selected"]))
        run("obsolete-references", ["rg", "-n", "-F", "-f", str(patterns), "src", "include", "config", "tools", ".agents", "docs", ".pi/workflows", "CMakeLists.txt", "README.md"], (1,))
        run("compile-commands", [str(root / ".venv/bin/python"), "-c", 'from pathlib import Path\nfrom harness.common.lease import acquire_writer\nfrom harness.commands.compile_commands import main\nwith acquire_writer(Path.cwd()):\n raise SystemExit(main([]))'])
        for target in targets:
            result = run(target.replace("/", "-") + "-readiness", ["bin/harness", "analysis", "readiness", target])
            ready = json.loads(result["stdout"])
            assert ready["ready"] and ready["index_ready"] and ready["stale_facts"]["count"] == 0
        inventory_data = (root / "build/cmake/bof3-inventory.json").read_bytes()
        inventory = json.loads(inventory_data)
        (out / "bof3-inventory.json").write_bytes(inventory_data)
        database_data = (root / "compile_commands.json").read_bytes()
        database = json.loads(database_data)
        (out / "compile_commands.json").write_bytes(database_data)
        database_sources = [str(Path(row["file"]).resolve()) for row in database]
        assert len(database_sources) == len(set(database_sources))
        by_source = {name: row for name, row in zip(database_sources, database)}
        desired = {unit["source"] for unit in plan["normal_build_units"]}
        obsolete = {m["old_source"] for m in plan["selected"]}
        graph = root / "build/cmake/build.ninja"
        graph_before = graph.stat()
        digest = hashlib.sha256()
        seen = set()
        with graph.open("rb") as stream, (out / "scoped-ninja-lines.txt").open("wb") as retained:
            for line in stream:
                digest.update(line)
                assert not any(old.encode() in line for old in obsolete), "Obsolete source in Ninja"
                hits = {name for name in desired if name.encode() in line}
                if hits:
                    seen.update(hits)
                    retained.write(line)
        assert all(getattr(graph.stat(), k) == getattr(graph_before, k) for k in ("st_dev", "st_ino", "st_mode", "st_nlink", "st_size", "st_mtime_ns", "st_ctime_ns")) and seen == desired
        for unit in plan["normal_build_units"]:
            source = unit["source"]
            assert source in inventory["sources"] and inventory["sources"][source] == (unit["metadata_member_count"] > 1)
            assert inventory["profiles"][source] == unit["effective_settings"]
            row = by_source[str(root / source)]
            arguments = list(unit["configured_arguments"])
            arguments[-1] = str((root / "build" / source).with_suffix(".o"))
            assert row["directory"] == str(root) and row["arguments"] == arguments
        for old in obsolete:
            assert old not in inventory["sources"] and old not in inventory["profiles"] and str(root / old) not in by_source
        summary["discovery"] = {"normal_TUs": 22, "old_paths_absent": 11, "inventory_sha256": sha(inventory_data), "root_database_sha256": sha(database_data), "ninja_sha256": digest.hexdigest(), "ninja_size": graph_before.st_size, "profiles_and_arguments_preserved": True}
        summary["foreign_battle_after"] = battle_endpoint()
        summary["foreign_battle_boundary"] = "Endpoint observations only; no continuous preservation or native approval of the foreign source. All target-native inputs retain their guards."
        assert directory_identities() == baseline["foreign_directory_identities"]
        guard()
        summary["terminal_input_states"] = states
    summary.update(status="local checks/builds/discovery complete; independent acceptance pending", normal_builds=11, live_exact_members=11, complete_original_bytes=340, guards_and_index_closed=True)
    save()
except BaseException as error:
    summary.update(status="failed", error=f"{type(error).__name__}: {error}")
    if isinstance(error, ProcessCleanupError):
        summary["cleanup"] = "unconfirmed; stop native work"
        summary["cleanup_observation"] = error.observed
    save()
    raise
