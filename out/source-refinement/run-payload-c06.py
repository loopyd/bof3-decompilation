"""Run the five public C06 deliveries and retain their actual process results."""
import json
from pathlib import Path
from harness.common.process import run_bounded

root = Path.cwd()
out = root / "out/source-refinement/payload-c06"
assert not out.exists()
targets = ["emi/bmagic/magic151/03", "emi/bmagic/magic154/03", "emi/bmagic/magic161/03", "emi/boss/boss001/13", "emi/boss/boss002/16"]
outputs = [root / "out/payloads" / ("c06-" + target.replace("/", "-")) for target in targets]
assert all(not path.exists() for path in outputs)
out.mkdir()
for target, output in zip(targets, outputs):
    label = target.replace("/", "-")
    argv = ["bin/harness", "build", "payload", target, "--output-dir", str(output.relative_to(root)), "--json"]
    print(" ".join(argv), flush=True)
    with (out / (label + ".stdout")).open("wb") as stdout, (out / (label + ".stderr")).open("wb") as stderr:
        result = run_bounded(root, argv, output_limit=64*1024*1024, on_output=lambda stream, chunk: (stdout if stream == "stdout" else stderr).write(chunk))
    (out / (label + ".process.json")).write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    if result["failure"] or result["exit_code"]:
        raise RuntimeError(f"{target}: public delivery failed; retain outputs and inspect actual process result")
    print(f"{target}: completed", flush=True)
