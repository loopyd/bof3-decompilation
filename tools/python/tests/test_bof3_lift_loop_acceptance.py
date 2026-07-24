from __future__ import annotations

import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
AGENT = ROOT / ".pi" / "agents" / "bof3-reverse.md"
PROTOCOL = ROOT / ".pi" / "skills" / "bof3-lift-loop" / "references" / "MISSION_PROTOCOL.md"


def _frontmatter(path: Path) -> dict[str, str]:
    lines = path.read_text(encoding="utf-8").splitlines()
    assert lines[0] == "---"
    end = lines.index("---", 1)
    return dict(line.split(": ", 1) for line in lines[1:end] if ": " in line)


def test_reverse_agent_accepts_exact_or_restored_escalation() -> None:
    frontmatter = _frontmatter(AGENT)
    acceptance = json.loads(frontmatter["acceptance"])

    assert frontmatter["completionGuard"] == "false"
    assert acceptance["level"] == "checked"
    assert acceptance["criteria"] == [
        "Produce either a byte-matched exact lift or an evidence-backed escalation with all mission edits restored, without widening scope."
    ]
    assert acceptance["evidence"] == [
        "changed-files",
        "tests-added",
        "commands-run",
        "validation-output",
        "residual-risks",
        "no-staged-files",
    ]
    assert ".pi/skills/bof3-lift-loop/references/MISSION_PROTOCOL.md" in AGENT.read_text(
        encoding="utf-8"
    )
    assert ".agents/skills/bof3-lift-loop" not in AGENT.read_text(encoding="utf-8")


def test_reverse_protocol_requires_truthful_escalation_evidence() -> None:
    text = PROTOCOL.read_text(encoding="utf-8")

    assert "```acceptance-report" in text
    assert "**Exact:** byte match passed" in text
    assert "**Escalated:** the mission JSON says `status: \"escalated\"`" in text
    assert 'both `files_changed` and `changedFiles` are `[]`' in text
    assert "restoration commands" in text
    assert "no-retained-changes `diffSummary`" in text
    assert "missing fence, missing evidence, or an\nescalation falsely claimed as exact remains a failed acceptance report" in text
