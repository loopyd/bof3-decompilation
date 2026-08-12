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
        assert "const MAX_ATTEMPTS = 10" in rendered
        assert "What other experiments could we try" in rendered
        assert "x.terminal = !restores[i] || !restores[i].ok" in rendered
        assert "x.restore = null" in rendered
        assert "--require-improvement --soft-no-improvement" in rendered
        assert "checkpointImprovedOf(x.checkpoint)" in rendered
        assert "const restored = lanes.filter" not in rendered
        assert "No new experiments and no edits" not in rendered
        syntax.write_text("async function lane(){\n" + rendered + "\n}\n")
        run("node", "--check", str(syntax))
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
