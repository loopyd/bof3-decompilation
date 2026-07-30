from __future__ import annotations

import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
AGENT = ROOT / ".pi" / "agents" / "bof3-reverse.md"
REVIEW_AGENT = ROOT / ".pi" / "agents" / "bof3-review.md"
SKILL = ROOT / ".pi" / "skills" / "bof3-re" / "SKILL.md"
PROTOCOL = ROOT / ".pi" / "skills" / "bof3-lift-loop" / "references" / "MISSION_PROTOCOL.md"
REVIEW_CHECKLIST = ROOT / ".pi" / "skills" / "bof3-lift-loop" / "references" / "REVIEW_CHECKLIST.md"
MATCHING = ROOT / "docs" / "matching.md"
POLICY_SOURCES = [
    ROOT / "AGENTS.md",
    ROOT / "include" / "base" / "barrier.h",
    ROOT / "docs" / "memory-api.md",
    ROOT / "docs" / "matching-playbook.md",
    ROOT / "docs" / "plans" / "implementation-roadmap.md",
    ROOT / ".agents" / "skills" / "bof3-re" / "SKILL.md",
    ROOT / ".agents" / "skills" / "bof3-lift-loop" / "references" / "MISSION_PROTOCOL.md",
    ROOT / ".agents" / "skills" / "bof3-lift-loop" / "references" / "REVIEW_CHECKLIST.md",
]


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


def test_register_pin_autonomy_stays_local_and_evidence_gated() -> None:
    texts = [
        AGENT.read_text(encoding="utf-8"),
        REVIEW_AGENT.read_text(encoding="utf-8"),
        SKILL.read_text(encoding="utf-8"),
        PROTOCOL.read_text(encoding="utf-8"),
        REVIEW_CHECKLIST.read_text(encoding="utf-8"),
        MATCHING.read_text(encoding="utf-8"),
        *(path.read_text(encoding="utf-8") for path in POLICY_SOURCES),
    ]
    for text in texts:
        assert "entry-register" in text
        assert "MATCHING_AID" in text
    agent = AGENT.read_text(encoding="utf-8")
    review_agent = REVIEW_AGENT.read_text(encoding="utf-8")
    skill = SKILL.read_text(encoding="utf-8")
    protocol = PROTOCOL.read_text(encoding="utf-8")
    checklist = REVIEW_CHECKLIST.read_text(encoding="utf-8")
    matching = MATCHING.read_text(encoding="utf-8")
    assert "Make one bounded local\nexperiment" in agent
    assert "live exact byte-match" in agent
    assert "one bounded local experiment" in review_agent
    assert "live exact match" in review_agent
    assert "independent review" in skill
    assert "live exact match" in protocol
    assert "live exact match" in checklist
    assert "independent review" in matching
    assert "generic matching macro" in skill
    assert "numeric pin" in skill
    for text in texts:
        assert "independent review" in " ".join(text.split())
    fallback = (ROOT / "include" / "bof3" / "asm.h").read_text(encoding="utf-8")
    for text in (skill, protocol, matching, fallback):
        assert "INCLUDE_ASM" in text
        assert "user approval" in text


def test_reverse_protocol_requires_truthful_escalation_evidence() -> None:
    text = PROTOCOL.read_text(encoding="utf-8")

    assert "```acceptance-report" in text
    assert "**Exact:** byte match passed" in text
    assert "**Escalated:** the mission JSON says `status: \"escalated\"`" in text
    assert 'both `files_changed` and `changedFiles` are `[]`' in text
    assert "restoration commands" in text
    assert "no-retained-changes `diffSummary`" in text
    assert "missing fence, missing evidence, or an\nescalation falsely claimed as exact remains a failed acceptance report" in text
