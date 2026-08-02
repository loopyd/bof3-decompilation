from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
AGENT = ROOT / ".pi" / "agents" / "bof3-cleanup.md"


def test_cleanup_agent_preserves_lift_identity_and_requires_evidence() -> None:
    text = AGENT.read_text(encoding="utf-8")

    assert "two independent corroborators" in text
    assert "never rename/move `func_XXXXXXXX.c`" in text
    assert "rename a Splat function boundary" in text
    assert "`audit PATHS...`" in text
    assert "`docs PATHS...`" in text
    assert "docs/plans/" in text
    assert "Do not stage, commit, push, reset, clean, checkout" in text
