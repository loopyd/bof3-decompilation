from __future__ import annotations

import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
AGENT = ROOT / ".pi" / "agents" / "bof3-reverse.md"
REVIEW_AGENT = ROOT / ".pi" / "agents" / "bof3-review.md"
SKILL = ROOT / ".pi" / "skills" / "bof3-re" / "SKILL.md"
PROTOCOL = ROOT / ".pi" / "skills" / "bof3-re" / "references" / "REVERSE" / "MISSION_PROTOCOL.md"
REVIEW_CHECKLIST = ROOT / ".pi" / "skills" / "bof3-re" / "references" / "REVIEW" / "REVIEW_CHECKLIST.md"
SHARING_NONMATCHES = ROOT / ".pi" / "skills" / "bof3-re" / "references" / "REVIEW" / "SHARING_NONMATCHES.md"
MATCHING = ROOT / "docs" / "matching.md"
POLICY_SOURCES = [
    ROOT / "AGENTS.md",
    ROOT / "include" / "base" / "barrier.h",
    ROOT / "docs" / "memory-api.md",
    ROOT / "docs" / "matching-playbook.md",
    ROOT / "docs" / "plans" / "implementation-roadmap.md",
    ROOT / ".agents" / "skills" / "bof3-re" / "SKILL.md",
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
    assert ".pi/skills/bof3-re/references/REVERSE/MISSION_PROTOCOL.md" in AGENT.read_text(
        encoding="utf-8"
    )
    assert ".agents/skills/" not in AGENT.read_text(encoding="utf-8")


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


def test_agents_receive_project_knowledge_and_reviewer_records_only_durable_facts() -> None:
    reverse = AGENT.read_text(encoding="utf-8")
    review = REVIEW_AGENT.read_text(encoding="utf-8")
    context = (ROOT / ".pi/skills/bof3-re/scripts/agent-context.py").read_text(
        encoding="utf-8"
    )
    checklist = REVIEW_CHECKLIST.read_text(encoding="utf-8")

    for text in (reverse, review, checklist):
        assert "LESSONS.md" in text
        assert "docs/specs/**/*.md" in text
    assert "LESSONS.md" in context
    assert 'rglob("*.md")' in context
    assert "tools: read,grep,find,ls,bash,edit,contact_supervisor" in review
    assert "Do not edit lift source, headers, maps, Splat, bindings" in review
    assert "durable, evidence-backed, cross-function" in review
    assert "selector, address, byte percentage" in review
    assert "do not\nedit project knowledge docs" in reverse
    assert "knowledge_paths" in context


def test_partial_share_uses_layout_eligibility_not_lift_evidence() -> None:
    text = (ROOT / ".pi" / "skills" / "bof3-lift-loop" / "SKILL.md").read_text(
        encoding="utf-8"
    )

    assert "bin/scratchpad share SELECTOR" in text
    assert "Missing ABI,\ncall ownership, analyzer confidence" in text
    assert "does **not** make\nan otherwise valid function unshareable" in text
    sharing = SHARING_NONMATCHES.read_text(encoding="utf-8")
    assert "Missing ABI, call ownership" in sharing
    assert "does not make an\notherwise qualifying function unshareable" in sharing


def test_reverse_protocol_requires_truthful_escalation_evidence() -> None:
    text = PROTOCOL.read_text(encoding="utf-8")

    assert "Append the required fenced `acceptance-report`" in text
    assert "Exact requires live byte" in text
    assert "Escalated requires empty changed-file lists" in text
    assert "no retained-change summary" in text
