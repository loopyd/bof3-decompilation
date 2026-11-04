"""Run the five public C12 deliveries and retain their actual process results."""
import json
import os
from pathlib import Path
from harness.common.process import run_bounded

root = Path.cwd()
out = root / "out/source-refinement/payload-c12"
assert not out.exists()
targets = ["emi/boss/boss040/16", "emi/boss/boss042/16", "emi/boss/boss049/16", "emi/boss/boss050/16", "emi/boss/boss051/16"]
outputs = [root / "out/payloads" / ("c12-" + target.replace("/", "-")) for target in targets]
assert all(not path.exists() for path in outputs)
environment=dict(os.environ)
prefix=Path("/home/koija/.codex/tmp/arg0-linux")
environment["PATH"]=os.pathsep.join(entry for entry in environment["PATH"].split(os.pathsep) if not Path(entry).is_relative_to(prefix))
out.mkdir()
for target, output in zip(targets, outputs):
    label = target.replace("/", "-")
    argv = ["bin/harness", "build", "payload", target, "--output-dir", str(output.relative_to(root)), "--json"]
    print(" ".join(argv), flush=True)
    with (out / (label + ".stdout")).open("wb") as stdout, (out / (label + ".stderr")).open("wb") as stderr:
        result = run_bounded(root, argv, env=environment, output_limit=64*1024*1024, on_output=lambda stream, chunk: (stdout if stream == "stdout" else stderr).write(chunk))
    (out / (label + ".process.json")).write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    if result["failure"] or result["exit_code"]:
        raise RuntimeError(f"{target}: public delivery failed; retain outputs and inspect actual process result")
    print(f"{target}: completed", flush=True)
