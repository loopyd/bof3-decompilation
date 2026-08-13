#!/usr/bin/env python3
"""Focused orchestration and parent-managed worktree self-check."""

import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[4]
SCRIPTS = ROOT / ".pi/skills/bof3-lift-loop/scripts"
SELECTOR = "emi/etc/shop/00@0x801DDFB0"


def run(*args: str, check: bool = True) -> subprocess.CompletedProcess[str]:
    return subprocess.run(args, cwd=ROOT, text=True, capture_output=True, check=check)


def rejected(*args: str) -> None:
    assert run(*args, check=False).returncode != 0


def main() -> int:
    key = "orchestration-self-check"
    subprocess.run(("python3", str(SCRIPTS / "lane-worktree.py"), "remove", "--key", key), cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    with tempfile.TemporaryDirectory() as directory:
        output = Path(directory) / "lane.js"
        common = ("--selector", SELECTOR, "--run-key", key, "--output", str(output))
        run("python3", str(SCRIPTS / "render-workflow.py"), "render", *common)
        assert json.loads(run("python3", str(SCRIPTS / "render-workflow.py"), "verify", *common).stdout)["verified"]
        syntax = Path(directory) / "syntax.js"
        rendered = output.read_text()
        syntax.write_text("async function lane(){\n" + rendered + "\n}\n")
        run("node", "--check", str(syntax))
        behavior = Path(directory) / "behavior.js"
        behavior.write_text(
            "const saved = {}, calls = [];\n"
            "const state = {get: async k => saved[k], set: async (k,v) => {saved[k]=v;}};\n"
            "const gate = metric => ({ok:true,results:[{acceptance:{verifyRuns:[{stdout:JSON.stringify({accepted:true,improved:true,current:{metric}})}]}}]});\n"
            "const measured = {ok:true,output:JSON.stringify({status:'exact',match_percent:100,files_changed:[]})};\n"
            "const runs = {run: async (k,o) => {calls.push(k); return k === 'baseline' || k === 'final-measure' ? measured : "
            "k === 'checkpoint-baseline' ? gate({match_percent:100,exact:true}) : "
            "k === 'integrate' ? {ok:true,results:[{acceptance:{verifyRuns:[{stdout:JSON.stringify({integrated:true,commit:'test'})}]}}]} : "
            "{ok:true,output:JSON.stringify({verdict:'pass'})};}};\n"
            "async function lane(){\n" + rendered + "\n}\n"
            "lane().then(v => console.log(JSON.stringify({result:v,state:saved,calls})));\n"
        )
        result = json.loads(run("node", str(behavior)).stdout)
        assert result["result"]["status"] == "integrated"
        assert result["result"]["attempt"] == 0
        assert result["result"]["bestScore"] == 100
        assert result["state"]["lane"]["status"] == "integrated"
        assert "cleanup" in result["calls"]
        assert "consolidation-review" in result["calls"]
        assert "integrate" in result["calls"]
        assert result["calls"].count("checkpoint-baseline") == 1
        assert not any(call.startswith("restore-") for call in result["calls"])
        state = json.loads(run("python3", str(SCRIPTS / "lane-worktree.py"), "create", "--key", key, "--selector", SELECTOR, "--allow-dirty").stdout)
        worktree = Path(state["worktree"])
        marker = worktree / "orchestration-self-check.txt"
        marker.write_text("shared cwd\n")
        manager = str(SCRIPTS / "lane-worktree.py")
        rejected("python3", manager, "export", "--key", key, "--selector", "wrong@0x00000000")
        forbidden = worktree / "build/orchestration-private.bin"
        forbidden.parent.mkdir(parents=True, exist_ok=True)
        forbidden.write_bytes(b"private")
        rejected("python3", manager, "export", "--key", key, "--selector", SELECTOR)
        forbidden.unlink()
        handoff = json.loads(run("python3", manager, "export", "--key", key, "--selector", SELECTOR).stdout)
        assert handoff["base"] == state["base"] and handoff["patch_sha256"]
        assert "orchestration-self-check.txt" in Path(handoff["patch"]).read_text()
        sentinel_key = "orchestration-unknown"
        sentinel = ROOT.parent / ".bof3-lift-worktrees" / sentinel_key
        sentinel.mkdir(parents=True, exist_ok=True)
        (sentinel / "keep").write_text("safe\n")
        rejected("python3", manager, "remove", "--key", sentinel_key)
        assert (sentinel / "keep").exists()
        (sentinel / "keep").unlink()
        sentinel.rmdir()
    run("python3", str(SCRIPTS / "lane-worktree.py"), "remove", "--key", key)
    print("orchestration self-check: OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
