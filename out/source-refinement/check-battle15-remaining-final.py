"""Check the applied Battle15 metadata and its normal target build."""

import argparse
import hashlib
import json
import os
from pathlib import Path

from harness.analysis.index import connect
from harness.common.inputs import InputBatch
from harness.common.process import run_bounded
from harness.reconstruction.objectfile import inspect_relocatable_object

root = Path(__file__).resolve().parents[2]
assert Path.cwd() == root
base = root / "out/source-refinement"
parser = argparse.ArgumentParser()
parser.add_argument("--post-summary-sha256", required=True)
args = parser.parse_args()
sha = lambda body: hashlib.sha256(body).hexdigest()
pre_path = base / "battle15-remaining-metadata-pre/summary.json"
post_path = base / "battle15-remaining-metadata-post/summary.json"
assert sha(pre_path.read_bytes()) == "c38ab86f8da071d6f91307ac0a14eebae67df1b1f5b0ecf619dbb480fed7d410"
assert sha(post_path.read_bytes()) == args.post_summary_sha256
pre, post = json.loads(pre_path.read_bytes()), json.loads(post_path.read_bytes())
assert post["guards_closed"] and post["member_count"] == 34 and post["complete_original_bytes"] == 1632
assert all(row["complete_pre_images_and_metrics_equal"] for row in post["members"])
members = {row["selector"]: row for row in pre["members"]}
assert len(members) == 34
out = base / "battle15-remaining-metadata-final-checks"
assert not out.exists()
out.mkdir()
summary = {"status": "running", "commands": [], "limits": "The 34 selected attached records only. Global validation and normal target build do not establish whole-target strict metadata attachment or runtime acceptance."}
environment = dict(os.environ)
environment["CMAKE_BUILD_PARALLEL_LEVEL"] = "1"


def save():
    (out / "summary.json").write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n")


try:
    with InputBatch(root) as inputs:
        states = {}

        def bind(name, digest=None):
            state, body = inputs.read(root / name, max_bytes=64 * 1024 * 1024, include_metadata=True, single_link=True)
            assert digest is None or state["sha256"] == digest, name
            states[name] = state
            return body

        bind(pre_path.relative_to(root).as_posix(), "c38ab86f8da071d6f91307ac0a14eebae67df1b1f5b0ecf619dbb480fed7d410")
        bind(post_path.relative_to(root).as_posix(), args.post_summary_sha256)
        # The PRE closure contains authored/proposed inputs and the immutable binary.
        # Generated comparison, assembly and object outputs remain owned by producers.
        expected = json.loads(bind("out/source-refinement/battle15-remaining-metadata-pre/terminal-input-states.json"))
        application = json.loads(bind(post["application_report"], post["application_sha256"]))
        assert application["guards_and_writer_closed"] and application["index_before"] == application["index_after"]
        expected.update(application["after_states"])
        expected[".git/index"] = application["index_after"]
        for name in [*expected, pre_path.relative_to(root).as_posix(), post_path.relative_to(root).as_posix(), Path(__file__).resolve().relative_to(root).as_posix()]:
            state, _ = inputs.read(root / name, max_bytes=64 * 1024 * 1024, include_metadata=True, single_link=True)
            if name in expected:
                assert state == expected[name], name
            states[name] = state

        def guard():
            for name, expected_state in states.items():
                state, _ = inputs.read(root / name, max_bytes=64 * 1024 * 1024, include_metadata=True, single_link=True)
                assert state == expected_state, name

        def run(label, argv, allowed=(0,)):
            guard()
            print(label, flush=True)
            result = run_bounded(root, argv, env=environment, output_limit=64 * 1024 * 1024)
            (out / (label + ".process.json")).write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
            (out / (label + ".stdout")).write_text(result["stdout"])
            (out / (label + ".stderr")).write_text(result["stderr"])
            summary["commands"].append({"label": label, "argv": argv, "exit_code": result["exit_code"], "failure": result["failure"]})
            save()
            assert result["failure"] is None and result["exit_code"] in allowed, label
            guard()
            return result

        target = "emi/battle/battle/15"
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
        report = json.loads(run("selected-metadata", ["bin/harness", "source", "metadata", "check", target, *members, "--json"])["stdout"])
        assert report["passed"] and not report["incomplete"] and not report["findings"]
        assert {row["selector"] for row in report["records"]} == set(members)
        for row in report["records"]:
            measured, before = row["measurement"], members[row["selector"]]["report"]
            assert not row["findings"] and measured["state"] == "live" and measured["status"] == "exact"
            for key in ("instruction_count", "original_size", "current_size"):
                assert measured[key] == before[key], (row["selector"], key)
            assert measured["match_percent"] == 100
        run("symbols", ["bin/harness", "source", "symbols", "check", target])
        run("splat", ["bin/harness", "source", "splat", target])
        run("build", ["bin/harness", "build", target])
        for member in members.values():
            path = root / "build" / Path(member["source"]).with_suffix(".o")
            body = path.read_bytes()
            image = inspect_relocatable_object(body, member["symbol"], limit=64 * 1024 * 1024)
            assert image.object_type == 1 and image.machine == 8
            assert image.entry_defined and image.entry_type == "func" and image.entry_size == member["original_size"]
        validation_path = out / "source-validation.json"
        run("source-validation", ["bin/harness", "source", "validate", "--out", str(validation_path.relative_to(root))])
        validation = json.loads(validation_path.read_bytes())
        owner, = [row for row in validation["targets"] if row["target"] == target]
        assert validation["coverage_error"] is None and owner["coverage_error"] is None and validation["lifts"]["invalid"] == 0
        for member in members.values():
            row, = [row for row in owner["functions"] if row["source"] == member["source"] and int(row["address"], 16) == member["address"]]
            assert row["status"] != "invalid"
        summary["actual_source_validation"] = {"global_lifts": validation["lifts"], "owning_target_lifts": owner["lifts"], "selected_noninvalid": 34}
        guard()
    summary.update(status="selected metadata and normal target gates complete; independent acceptance pending", selected_records=34, exact_records=34, complete_original_bytes=1632, guards_and_index_closed=True)
    save()
except BaseException as error:
    summary.update(status="failed; keep applied files and fix cause", error=f"{type(error).__name__}: {error}")
    save()
    raise
