import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
AUDIT = ROOT / ".pi/skills/agent-skill-compaction/scripts/audit.py"


def run(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        (sys.executable, str(AUDIT), *args),
        cwd=ROOT,
        text=True,
        capture_output=True,
    )


def test_compaction_audit_default_tree_and_self_baseline(tmp_path: Path) -> None:
    baseline = tmp_path / "baseline.json"
    assert run("--output", str(baseline)).returncode == 0
    result = run("--baseline", str(baseline), "--check")
    assert result.returncode == 0, result.stderr
    report = json.loads(result.stdout)
    assert report["totals"]["files"] >= 31
    assert not report["errors"]


def test_compaction_audit_rejects_bad_scope_and_markdown(tmp_path: Path) -> None:
    assert run(str(tmp_path / "missing"), "--check").returncode != 0
    plain = tmp_path / "plain.txt"
    plain.write_text("not markdown")
    assert run(str(plain), "--check").returncode != 0

    bad_link = tmp_path / "bad-link.md"
    bad_link.write_text("# X\n\n[bad][missing]\n")
    assert run(str(bad_link), "--check").returncode != 0

    bad_fence = tmp_path / "bad-fence.md"
    bad_fence.write_text("# X\n\n````py\nx\n```\n")
    assert run(str(bad_fence), "--check").returncode != 0


def test_compaction_audit_rejects_baseline_scope_drift(tmp_path: Path) -> None:
    baseline = tmp_path / "baseline.json"
    assert run(".pi/agents", "--output", str(baseline)).returncode == 0
    assert run(
        ".pi/agents", ".pi/skills", "--baseline", str(baseline), "--check"
    ).returncode != 0
