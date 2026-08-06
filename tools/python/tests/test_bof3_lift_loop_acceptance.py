from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[3]
AGENT = ROOT / ".pi" / "agents" / "bof3-reverse.md"
REVIEW_AGENT = ROOT / ".pi" / "agents" / "bof3-review.md"
SKILL = ROOT / ".pi" / "skills" / "bof3-re" / "SKILL.md"
PROTOCOL = (
    ROOT
    / ".pi"
    / "skills"
    / "bof3-re"
    / "references"
    / "REVERSE"
    / "MISSION_PROTOCOL.md"
)
REVIEW_CHECKLIST = (
    ROOT
    / ".pi"
    / "skills"
    / "bof3-re"
    / "references"
    / "REVIEW"
    / "REVIEW_CHECKLIST.md"
)
SHARING_NONMATCHES = (
    ROOT
    / ".pi"
    / "skills"
    / "bof3-re"
    / "references"
    / "REVIEW"
    / "SHARING_NONMATCHES.md"
)
MATCHING = ROOT / "docs" / "agents" / "matching.md"
POLICY_SOURCES = [
    ROOT / "AGENTS.md",
    ROOT / "include" / "base" / "barrier.h",
    ROOT / "docs" / "agents" / "memory-api.md",
    ROOT / "docs" / "agents" / "matching-playbook.md",
    ROOT / ".pi" / "skills" / "bof3-re" / "SKILL.md",
]


def _frontmatter(path: Path) -> dict[str, str]:
    lines = path.read_text(encoding="utf-8").splitlines()
    assert lines[0] == "---"
    end = lines.index("---", 1)
    return dict(line.split(": ", 1) for line in lines[1:end] if ": " in line)


def _flat(text: str) -> str:
    return " ".join(text.split())


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
    assert (
        ".pi/skills/bof3-re/references/REVERSE/MISSION_PROTOCOL.md"
        in AGENT.read_text(encoding="utf-8")
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
    assert "Make one bounded local experiment" in _flat(agent)
    assert "live exact byte-match" in _flat(agent)
    assert "one bounded local experiment" in _flat(review_agent)
    assert "live exact match" in _flat(review_agent)
    assert "independent review" in _flat(skill)
    assert "live exact match" in _flat(protocol)
    assert "live exact match" in _flat(checklist)
    assert "independent review" in _flat(matching)
    assert "generic matching macro" in _flat(skill)
    assert "numeric pin" in _flat(skill)
    for text in texts:
        assert "independent review" in " ".join(text.split())
    fallback = (ROOT / "include" / "bof3" / "asm.h").read_text(encoding="utf-8")
    for text in (skill, protocol, matching, fallback):
        assert "INCLUDE_ASM" in text
        assert "user approval" in text


def test_agents_receive_project_knowledge_and_reviewer_records_only_durable_facts() -> (
    None
):
    reverse = AGENT.read_text(encoding="utf-8")
    review = REVIEW_AGENT.read_text(encoding="utf-8")
    context = (ROOT / ".pi/skills/bof3-re/scripts/agent-context.py").read_text(
        encoding="utf-8"
    )
    checklist = REVIEW_CHECKLIST.read_text(encoding="utf-8")

    for text in (reverse, review, checklist):
        assert "docs/agents/lessons.md" in text
    for text in (reverse, review):
        assert "all `docs/specs/**/*.md`" not in text
    assert "docs/agents/lessons.md" in context
    assert 'rglob("*.md")' not in context
    assert "tools: read,grep,find,ls,bash,edit,contact_supervisor" in review
    assert "Do not edit lift source, headers, maps, Splat, bindings" in _flat(review)
    assert "durable, evidence-backed, cross-function" in _flat(review)
    assert "selector, address, byte percentage" in _flat(review)
    assert "do not edit project knowledge docs" in _flat(reverse)
    assert "knowledge_paths" in context


def test_partial_share_uses_layout_eligibility_not_lift_evidence() -> None:
    text = (ROOT / ".pi" / "skills" / "bof3-lift-loop" / "SKILL.md").read_text(
        encoding="utf-8"
    )

    assert "bin/scratchpad share SELECTOR" in _flat(text)
    assert "Missing ABI, call ownership, analyzer confidence" in _flat(text)
    assert "does **not** make an otherwise valid function unshareable" in _flat(text)
    sharing = SHARING_NONMATCHES.read_text(encoding="utf-8")
    assert "Missing ABI, call ownership" in _flat(sharing)
    assert "does not make an otherwise qualifying function unshareable" in _flat(sharing)


def test_reverse_protocol_requires_truthful_escalation_evidence() -> None:
    text = PROTOCOL.read_text(encoding="utf-8")

    assert "Append the required fenced `acceptance-report`" in _flat(text)
    assert "Exact requires live byte" in _flat(text)
    assert "Escalated requires empty changed-file lists" in _flat(text)
    assert "no retained-change summary" in _flat(text)


def test_lift_loop_gates_cleanup_with_fresh_match_and_review() -> None:
    text = (ROOT / ".pi" / "skills" / "bof3-lift-loop" / "SKILL.md").read_text(
        encoding="utf-8"
    )

    assert "`bof3-cleanup`" in text
    assert "cosmetic, evidence-preserving changes only" in _flat(text)
    assert "fresh live `byte-match` and dispatch a fresh `bof3-review`" in text
    assert "both must pass before the function stays eligible" in _flat(text)
    assert "reverted, never fixed forward" in _flat(text)
    cleanup = (ROOT / ".pi" / "agents" / "bof3-cleanup.md").read_text(encoding="utf-8")
    assert "cosmetic and evidence-preserving only" in _flat(cleanup)
    assert "post-cleanup live `bin/byte-match TARGET@0xADDRESS` must pass" in _flat(cleanup)


def test_usage_docs_require_explicit_commit_authorization() -> None:
    text = (ROOT / "docs" / "usage.md").read_text(encoding="utf-8")

    assert (
        "commits only reviewed exact\nlifts after explicit user commit authorization"
        in text
    )
    index = (ROOT / "docs" / "index.md").read_text(encoding="utf-8")
    assert "Agent-facing references:" not in index
    assert "## Agent operating references" in index


def test_agent_context_reading_order_and_agents_role() -> None:
    text = (ROOT / ".pi/skills/bof3-re/scripts/agent-context.py").read_text(
        encoding="utf-8"
    )

    soul = text.index('"SOUL.md"')
    agents_md = text.index('"AGENTS.md"')
    standards = text.index('"docs/agents/CODING_STANDARDS.md"')
    assert soul < agents_md < standards
    assert '"agents": ()' in text
    assert 'default="agents"' in text
    assert 'if args.role == "agents":' in text
    assert "def roster(root: Path)" in text
    agents_doc = (ROOT / "AGENTS.md").read_text(encoding="utf-8")
    assert "agent-context.py agents" in agents_doc
    assert (ROOT / "docs" / "agents" / "CODING_STANDARDS.md").is_file()


def test_agent_context_workflow_roles_bounded() -> None:
    text = (ROOT / ".pi/skills/bof3-re/scripts/agent-context.py").read_text(
        encoding="utf-8"
    )
    assert "WORKFLOW = {" in text
    for role in (
        "classifier",
        "context-builder",
        "oracle",
        "planner",
        "researcher",
        "reviewer",
        "scout",
        "worker",
    ):
        assert f'"{role}"' in text
    agents_doc = (ROOT / "AGENTS.md").read_text(encoding="utf-8")
    assert "## Completion" in agents_doc


def test_agent_context_qwen_roles_stay_bounded() -> None:
    script = ROOT / ".pi/skills/bof3-re/scripts/agent-context.py"
    for definition in sorted((ROOT / ".pi/agents").glob("*.md")):
        front = definition.read_text(encoding="utf-8").split("---")[1]
        fields = dict(
            line.split(": ", 1) for line in front.splitlines() if ": " in line
        )
        if "qwen" not in fields.get("model", ""):
            continue
        name = fields["name"]
        result = subprocess.run(
            (sys.executable, str(script), name),
            cwd=ROOT,
            capture_output=True,
            text=True,
            check=True,
        )
        assert len(result.stdout.encode()) < 12_000, name
        assert result.stdout.count("=====") <= 4, name


def test_pi_context_files_stay_compact() -> None:
    files = sorted((ROOT / ".pi/agents").glob("*.md"))
    files += sorted((ROOT / ".pi/skills").glob("*/SKILL.md"))
    files += sorted((ROOT / ".pi/skills/bof3-re/references").glob("*/*.md"))
    total = sum(len(path.read_bytes()) for path in files)
    assert total <= 52_200, f".pi context files re-inflated: {total} bytes"
    docs = sorted((ROOT / "docs" / "agents").glob("*.md"))
    docs_total = sum(len(path.read_bytes()) for path in docs)
    assert docs_total <= 49_500, f"docs/agents re-inflated: {docs_total} bytes"
