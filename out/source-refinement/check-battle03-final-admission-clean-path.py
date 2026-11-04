"""Run supported final gates for the applied Battle03 metadata repair."""
import json
import os
import runpy
from pathlib import Path
from harness.common.inputs import InputBatch
from harness.common.process import run_bounded

root = Path.cwd()
base = root / "out/source-refinement"
out = base / "battle03-final-admission-checks-clean-path"
assert not out.exists()
plan = json.loads((base / "battle03-final-admission-clean-plan.json").read_text())
target = "emi/battle/battle/03"
selectors = [m["selector"] for m in plan["members"]]
commands = [
    ("metadata", ["bin/harness", "source", "metadata", "check", target, *selectors, "--json"]),
    ("symbols", ["bin/harness", "source", "symbols", "check", target]),
    ("splat", ["bin/harness", "source", "splat", target]),
    ("normal-build", ["bin/harness", "build", target]),
]
proof = runpy.run_path(str(base / "check-battle03-program-equivalence-final.py"))
environment = dict(os.environ)
environment["PATH"] = proof["filtered"]
out.mkdir()
summary = {"status": "running", "commands": [], "limit": "Independent final acceptance pending. No runtime or source-only reconstruction claim."}
def save():
    (out / "summary.json").write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n")
save()
try:
    with InputBatch(root) as inputs:
        def guard():
            for name, expected in plan["current_input_states"].items():
                state, _ = inputs.read(root / name, max_bytes=64*1024*1024, include_metadata=True, single_link=True)
                if state != expected:
                    raise ValueError(f"Reviewed input changed: {name}")
        guard()
        for label, argv in commands:
            print(" ".join(argv), flush=True)
            with (out / (label + ".stdout")).open("wb") as stdout, (out / (label + ".stderr")).open("wb") as stderr:
                result = run_bounded(root, argv, env=environment, output_limit=64*1024*1024, on_output=lambda stream, chunk: (stdout if stream == "stdout" else stderr).write(chunk))
            (out / (label + ".process.json")).write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
            summary["commands"].append({"label": label, "argv": argv, "exit_code": result["exit_code"], "failure": result["failure"]})
            save()
            if result["failure"] or result["exit_code"]:
                raise RuntimeError(f"{label} failed; retain batch and actual outputs")
            guard()
    summary.update(status="all public gates complete", continuous_inputs_closed=True)
    save()
except BaseException as error:
    summary.update(status="failed", error=f"{type(error).__name__}: {error}")
    save()
    raise
