"""Refresh the stale SHOP snapshot and finish the unreached delivery gates."""
import argparse
from contextlib import closing
import hashlib
import json
import os
from pathlib import Path

from harness.build.compilation import configured_command
from harness.build.dependencies import IncludeSnapshot
from harness.common.inputs import InputBatch
from harness.common.process import ProcessCleanupError, run_bounded

ROOT = Path(__file__).resolve().parents[2]
BASE = "out/source-refinement/"
PLAN = BASE + "shop00-boundary-post-plan.json"
POST = BASE + "shop00-boundary-post/"
OUT = ROOT / (BASE + "shop00-boundary-final-followup")
PAYLOAD = "out/payloads/shop00-boundary"
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
    for option in ("post-plan-sha256", "application-sha256", "post-summary-sha256"):
        parser.add_argument("--" + option, required=True)
    args = parser.parse_args()
    require(Path.cwd() == ROOT and not OUT.exists() and not (ROOT / PAYLOAD).exists(), "Run at root with fresh outputs")
    OUT.mkdir()
    summary = {"status": "running", "commands": [], "limits": "SHOP52B successor gates only. All143 normal source/profile/include inputs remain guarded. Three explicitly plan-pinned generated Splat ASMs are verified in closed phases before/after their producer calls; their mutation metadata is not an authored-input guard across regeneration. Retained partials keep their existing source progress. Public child producers own writer leases. No runtime or replica delivery claimed."}
    save(OUT / "summary.json", summary)
    try:
        with InputBatch(ROOT) as inputs:
            states = {}
            def bind(name):
                if Path(name).is_absolute():
                    name = str(Path(name).relative_to(ROOT))
                require(".." not in Path(name).parts, "Escaped input")
                state, body = inputs.read(ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True)
                require(name not in states or states[name] == state, f"Input changed: {name}")
                states[name] = state
                return body
            recovery_path = BASE + "shop00-boundary-final-continuation-plan.json"
            recovery = json.loads(bind(recovery_path))
            require(sha(bind(recovery_path)) == "f30cedee0b7736a5abccf751309f2994a498a087baa36b4da7529ea9bde47a99", "Reviewed continuation changed")
            for name, digest in recovery["predecessor_artifacts"].items():
                require(sha(bind(name)) == digest, "Retained predecessor changed: " + name)
            previous = BASE + "shop00-boundary-final-checks/"
            previous_summary = json.loads(bind(previous + "summary.json"))
            require(previous_summary["error"] == recovery["expected_previous_error"], "Unexpected predecessor failure")
            require([row["label"] for row in previous_summary["commands"]] == recovery["passed_command_labels"] and all(row["exit_code"] == 0 and row["failure"] is None for row in previous_summary["commands"]), "Passed predecessor phases changed")
            previous_validation = json.loads(bind(previous + "source-validation.json"))
            require(previous_validation["coverage_error"] == recovery["expected_coverage_error"] and previous_validation["lifts"]["invalid"] == 0, "Expected stale recipe failure changed")
            for name, expected in json.loads(bind(previous + "starting-input-states.json")).items():
                bind(name)
                require(states[name] == expected, "Retained predecessor input changed: " + name)
            def verify_normal_objects(label):
                observed = {}
                with InputBatch(ROOT) as outputs:
                    for name, expected in recovery["normal_object_states"].items():
                        state, _ = outputs.read(ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True)
                        require(state == expected, "Accepted normal object changed: " + name)
                        observed[name] = state
                require(len(observed) == 143, "Normal object closure changed")
                save(OUT / (label + "-normal-object-states.json"), observed)
            verify_normal_objects("starting")
            summary["accepted_predecessor"] = {"recovery_plan": recovery_path, "recovery_plan_sha256": "f30cedee0b7736a5abccf751309f2994a498a087baa36b4da7529ea9bde47a99", "retained_passed_commands": recovery["passed_command_labels"], "normal_objects": 143}
            plan = json.loads(bind(PLAN))
            require(sha(bind(PLAN)) == args.post_plan_sha256, "Applied plan changed")
            app = json.loads(bind(plan["application_report"]))
            require(sha(bind(plan["application_report"])) == args.application_sha256 == plan["application_sha256"] and app["guards_and_writer_closed"], "Application changed")
            post = json.loads(bind(POST + "summary.json"))
            require(sha(bind(POST + "summary.json")) == args.post_summary_sha256 and post["status"] == "complete POST captures; independent acceptance pending" and post["guards_and_writer_closed"] and post["member_count"] == 1 and post["complete_original_bytes"] == 52 and post["plan_sha256"] == args.post_plan_sha256 and post["accepted_pre_sha256"] == plan["accepted_pre_sha256"], "Complete POST required")
            pre_path = BASE + "shop00-boundary-pre/summary.json"
            require(sha(bind(pre_path)) == plan["accepted_pre_sha256"], "Accepted PRE changed")
            for name, digest in ((app["pre_review"], app["pre_review_sha256"]), (app["foreign_report"], app["foreign_report_sha256"])):
                require(sha(bind(name)) == digest, "Independent PRE/foreign report changed")
            generated = {name: expected for name, expected in plan["current_input_states"].items() if name.startswith("out/splat/")}
            require(set(generated) == {"out/splat/emi/etc/shop/00/asm/" + name + ".s" for name in ("func_801E2F04", "func_801E3284", "func_801E37F4")}, "Declared producer output closure changed")
            # These three original ASMs are Splat outputs, not continuous authored inputs.
            def verify_generated(label):
                observed = {}
                with InputBatch(ROOT) as outputs:
                    for name, expected in generated.items():
                        state, body = outputs.read(ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True)
                        require(state is not None and body is not None and state["sha256"] == expected["sha256"], f"Generated original ASM bytes changed: {name}")
                        observed[name] = state
                save(OUT / (label + "-generated-states.json"), observed)
            verify_generated("before")
            for expected_states in (plan["current_input_states"], plan["foreign_states"]):
                for name, expected in expected_states.items():
                    if name not in generated:
                        bind(name)
                        require(states[name] == expected, f"Applied/current/foreign state changed: {name}")
            index = bind(".git/index")
            require(states[".git/index"] == post["runtime_index_state"] == plan["runtime_index_state"] == app["runtime_index_state"], "Current index differs from POST")
            (OUT / "index-pre").write_bytes(index)
            bind(Path(__file__).resolve().relative_to(ROOT).as_posix())
            member, = plan["members"]
            proof, = post["members"]
            for suffix in ("original.bin", "current.bin"):
                require(bind(proof["saved_at"] + "/" + suffix) == bind(member["original_path"]), "Saved POST images changed")
            require(sha(bind(proof["elf_path"])) == proof["elf_sha256"] and proof["size"] == 52 and proof["current_sha256"] == proof["original_sha256"] == member["original_sha256"], "Actual POST proof changed")
            sources = [ROOT / unit["source"] for unit in plan["normal_build_units"]]
            require(len(sources) == len(set(sources)) == 143, "Normal TU closure changed")
            with closing(IncludeSnapshot(ROOT, sources[0], forced=tuple(sources[1:]))) as includes:
                require(includes.describe_inputs() == plan["include_input_states"], "Include closure changed")
                for name, expected in includes.describe_inputs().items():
                    bind(name)
                    require(states[name] == expected, f"Include changed: {name}")
                for unit in plan["normal_build_units"]:
                    argv, _ = configured_command(ROOT, ROOT / unit["source"], Path(unit["configured_arguments"][-1]), allow_grouped=True)
                    require(argv == unit["configured_arguments"], "Normal configured profile changed")
                def guard():
                    includes.validate()
                    for name in list(states):
                        bind(name)
                environment = dict(os.environ)
                environment["CMAKE_BUILD_PARALLEL_LEVEL"] = "1"
                def run(label, argv, allowed=(0,)):
                    guard()
                    print(" ".join(argv), flush=True)
                    with (OUT / (label + ".stdout")).open("wb") as stdout, (OUT / (label + ".stderr")).open("wb") as stderr:
                        def retain(stream, chunk):
                            (stdout if stream == "stdout" else stderr).write(chunk)
                        result = run_bounded(ROOT, argv, env=environment, output_limit=LIMIT, on_output=retain)
                    save(OUT / (label + ".process.json"), result)
                    summary["commands"].append({"label": label, "argv": argv, "exit_code": result["exit_code"], "failure": result["failure"]})
                    save(OUT / "summary.json", summary)
                    require(not result["failure"] and result["exit_code"] in allowed, f"{label} failed; retain outputs and fix cause")
                    guard()
                    if label in ("source-splat", "normal-build", "build-payload"):
                        verify_generated(label)
                    return result
                save(OUT / "starting-input-states.json", states)
                target = plan["target"]
                status = json.loads(run("analysis-status", ["bin/harness", "analysis", "rz-project", "status", target, "--json"], (0, 1))["stdout"])
                require(status["target"] == target, "Analysis target changed")
                if not status["fresh"]:
                    run("analysis-refresh", ["bin/harness", "analysis", "rz-project", "analyze", target])
                run("analysis-index", ["bin/harness", "analysis", "index"])
                run("source-validation", ["bin/harness", "source", "validate", "--out", str(OUT / "source-validation.json")])
                validation = json.loads((OUT / "source-validation.json").read_bytes())
                target_validation, = [row for row in validation["targets"] if row["target"] == target]
                require(validation["coverage_error"] is None and target_validation["coverage_error"] is None and target_validation["lifts"]["invalid"] == 0, "Source validation incomplete/invalid")
                run("compile-commands", [str(ROOT / ".venv/bin/python"), "-c", "from pathlib import Path\nfrom harness.common.lease import acquire_writer\nfrom harness.commands.compile_commands import main\nwith acquire_writer(Path.cwd()):\n raise SystemExit(main([]))"])
                inventory = json.loads((ROOT / "build/cmake/bof3-inventory.json").read_bytes())
                database = json.loads((ROOT / "compile_commands.json").read_bytes())
                rows = {row["file"]: row for row in database}
                require(len(rows) == len(database), "Duplicate compilation database sources")
                desired = {u["source"] for u in plan["normal_build_units"]}
                seen = set()
                digest = hashlib.sha256()
                graph = ROOT / "build/cmake/build.ninja"
                before = graph.stat()
                with graph.open("rb") as stream, (OUT / "scoped-ninja-lines.txt").open("wb") as saved:
                    for line in stream:
                        digest.update(line)
                        hits = {name for name in desired if name.encode() in line}
                        if hits:
                            seen.update(hits)
                            saved.write(line)
                require(seen == desired and all(getattr(graph.stat(), key) == getattr(before, key) for key in ("st_dev", "st_ino", "st_mode", "st_nlink", "st_size", "st_mtime_ns", "st_ctime_ns")), "Normal graph discovery changed/incomplete")
                for unit in plan["normal_build_units"]:
                    name = unit["source"]
                    arguments = list(unit["configured_arguments"])
                    arguments[-1] = str((ROOT / "build" / name).with_suffix(".o"))
                    require(name in inventory["sources"] and inventory["sources"][name] == (len(unit["records"]) > 1) and inventory["profiles"][name] == unit["effective_settings"] and rows[str(ROOT / name)]["arguments"] == arguments and rows[str(ROOT / name)]["directory"] == str(ROOT), f"Normal source/profile discovery differs: {name}")
                summary["normal_discovery"] = {"normal_TUs": 143, "authored_TUs": 141, "metadata_free_support_TUs": 2, "ninja_sha256": digest.hexdigest(), "inventory_sha256": sha((ROOT / "build/cmake/bof3-inventory.json").read_bytes()), "root_database_sha256": sha((ROOT / "compile_commands.json").read_bytes())}
                verify_normal_objects("before-payload")
                payload = json.loads(run("build-payload", ["bin/harness", "build", "payload", target, "--output-dir", PAYLOAD, "--json"])["stdout"])
                require(json.loads(bind(PAYLOAD + "/report.json")) == payload and payload["schema"] == "harness.build-payload/v1" and payload["target"] == target and payload["whole_payload_equal"] and payload["size"] == 87660, "Payload report differs")
                actual = bind(payload["output"])
                original = bind(payload["original"])
                require(actual == original == bind(plan["original_binary"]["path"]) and sha(actual) == payload["sha256"] == plan["original_binary"]["sha256"], "Whole shipped/normalized/payload equality failed")
                intervals = sorted([(r["start"], r["end"], "compiled") for r in payload["compiled"]] + [(r["start"], r["end"], "retained") for r in payload["retained"]])
                cursor = 0
                for start, end, _ in intervals:
                    require(start == cursor and start < end <= len(actual), "Payload interval gap/overlap")
                    cursor = end
                require(cursor == len(actual) and payload["compiled_bytes"] == sum(r["end"] - r["start"] for r in payload["compiled"]) and payload["compiled_bytes"] + payload["retained_bytes"] == len(actual), "Payload totals differ")
                selected, = [r for r in payload["compiled"] if member["selector"] in r["owners"]]
                require(selected["start"] == selected["normalized_start"] == 10836 and selected["end"] == 10888 and selected["symbol"] == member["symbol"] and selected["source"] == member["source"] and selected["section"] is None and selected["sha256"] == member["original_sha256"], "Selected52B contribution missing")
                archive = payload["archive"]
                require(archive["whole_archive_equal"] and archive["entry_equal"] and bind(archive["output"]) == bind(archive["original"]) and sha(bind(archive["output"])) == archive["sha256"] and bind(PAYLOAD + "/entry.bin") == actual, "Whole archive/extracted entry equality failed")
                summary["payload"] = {"report": PAYLOAD + "/report.json", "report_sha256": sha(bind(PAYLOAD + "/report.json")), "compiled_bytes": payload["compiled_bytes"], "retained_bytes": payload["retained_bytes"], "selected_compiled_bytes": 52, "whole_payload_archive_entry_equal": True}
                guard()
                verify_generated("terminal")
                save(OUT / "terminal-input-states.json", states)
        summary.update(status="SHOP52B cache repair, retained143-TU build,discovery and fresh payload complete; independent acceptance pending", guards_and_children_closed=True)
        save(OUT / "summary.json", summary)
    except BaseException as error:
        summary.update(status="failed; retain applied batch and fix cause", error=f"{type(error).__name__}: {error}")
        if isinstance(error, ProcessCleanupError):
            summary.update(cleanup="unconfirmed; stop native work", cleanup_observation=error.observed)
        save(OUT / "summary.json", summary)
        raise


if __name__ == "__main__":
    main()
