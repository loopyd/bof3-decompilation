from pathlib import Path

import yaml


ROOT = Path(__file__).resolve().parents[3]
AGENT = ROOT / ".pi" / "agents" / "bof3-cleanup.md"
AGENTS_DIR = ROOT / ".pi" / "agents"


def test_cleanup_agent_preserves_lift_identity_and_requires_evidence() -> None:
    text = AGENT.read_text(encoding="utf-8")

    assert "two independent corroborators" in text
    assert "never rename/move `func_XXXXXXXX.c`" in text
    assert "rename a Splat function boundary" in text
    assert "`audit PATHS...`" in text
    assert "`docs PATHS...`" in text
    assert "docs/plans/" in text
    assert "Do not stage, commit, push, reset, clean, checkout" in text


def test_every_agent_front_matter_is_a_yaml_mapping_with_identity() -> None:
    agents = sorted(AGENTS_DIR.glob("*.md"))
    assert agents
    for path in agents:
        lines = path.read_text(encoding="utf-8").splitlines()
        assert lines[0] == "---", path
        end = lines.index("---", 1)
        front_matter = yaml.safe_load("\n".join(lines[1:end]))
        assert isinstance(front_matter, dict), path
        assert front_matter.get("name") == path.stem, path
        assert isinstance(front_matter.get("description"), str), path
