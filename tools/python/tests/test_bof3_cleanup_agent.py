from __future__ import annotations

from dataclasses import FrozenInstanceError, replace
from pathlib import Path

import pytest

from harness.context.bof3_cleanup import (
    SelectedSkill,
    cleanup_sections,
    parse_cleanup_request,
)


ROOT = Path(__file__).resolve().parents[3]
SELECTOR = "exe/logo@0x801CE758"


@pytest.mark.parametrize(
    ("tokens", "mode", "skill", "references"),
    (
        (
            ("symbol", "exe/logo", "old", "->", "new"),
            "symbol",
            "bof3-identity-maintenance",
            ("IDENTITY_TRANSACTIONS.md",),
        ),
        (
            ("type", "exe/logo", "old", "->", "new"),
            "type",
            "bof3-identity-maintenance",
            ("IDENTITY_TRANSACTIONS.md",),
        ),
        (
            ("repair", "exe/logo", "function:func_801CE758"),
            "repair",
            "bof3-identity-maintenance",
            ("IDENTITY_TRANSACTIONS.md",),
        ),
        (
            (
                "retained-lift",
                "exe/logo",
                SELECTOR,
                "exact",
                "exe/logo@function:func_801CE758",
            ),
            "retained-lift",
            "bof3-identity-maintenance",
            ("IDENTITY_TRANSACTIONS.md", "BYTE_SAFE_COSMETICS.md"),
        ),
        (
            ("relocate-batch", "exe/logo", "ui", SELECTOR),
            "relocate-batch",
            "bof3-identity-maintenance",
            ("SOURCE_RELOCATION.md",),
        ),
        (
            ("docs", "docs/usage.md"),
            "docs",
            "repo-documentation-repair",
            ("DOCUMENTATION_REPAIR.md",),
        ),
        (
            ("audit-target", "exe/logo"),
            "audit-target",
            "bof3-naming-evidence",
            ("NAMING_AUDIT_V3.md",),
        ),
    ),
)
def test_canonical_routes_load_exact_selected_body_and_direct_refs(
    tokens: tuple[str, ...], mode: str, skill: str, references: tuple[str, ...]
) -> None:
    request = parse_cleanup_request(tokens)
    reads: list[Path] = []

    def read(path: Path) -> bytes:
        reads.append(path)
        return path.read_bytes()

    sections = cleanup_sections(ROOT, request, read)
    assert request.mode == mode
    assert request.selected_skill.name == skill
    assert (
        tuple(Path(path).name for path in request.selected_skill.direct_references)
        == references
    )
    assert reads[0] == ROOT / f".pi/skills/{skill}/SKILL.md"
    assert tuple(path.name for path in reads[1:]) == references
    assert len(reads) == 1 + len(references)
    loaded_bytes = sum(len(section.text.encode()) for section in sections[1:])
    assert loaded_bytes <= 16_000
    assert f'"loaded_bytes": {loaded_bytes}' in sections[0].text
    unselected = {
        "bof3-identity-maintenance",
        "repo-documentation-repair",
        "bof3-naming-evidence",
    } - {skill}
    assert not any(any(name in str(path) for name in unselected) for path in reads)


def test_cleanup_request_is_frozen_and_retains_structured_state() -> None:
    request = parse_cleanup_request(
        (
            "retained-lift",
            "exe/logo",
            SELECTOR,
            "improved-partial",
            "data:D_801CE760",
            "exe/logo@function:func_801CE758",
        )
    )
    assert request.target == "exe/logo"
    assert str(request.selector) == "exe/logo@801ce758"
    assert request.state == "improved-partial"
    assert request.rows == (
        "data:D_801CE760",
        "exe/logo@function:func_801CE758",
    )
    with pytest.raises(FrozenInstanceError):
        request.mode = "repair"  # type: ignore[misc]


def test_parent_old_audit_normalizes_docs_only_with_warning() -> None:
    request = parse_cleanup_request(
        ("audit", "docs/usage.md", "docs/index.md"), parent_compatibility=True
    )
    assert request.mode == "docs"
    assert request.warning
    assert request.arguments == ("docs/usage.md", "docs/index.md")


@pytest.mark.parametrize(
    "tokens",
    (
        (),
        ("unknown",),
        ("audit", "docs/usage.md"),
        ("audit", "src/file.c"),
        ("docs", "src/file.c"),
        ("docs", "docs/../src/file.c"),
        ("docs", ".pi/agents/worker.md"),
        ("docs", "docs/missing.md"),
        ("audit-target", "exe/missing"),
        ("audit-target", "exe/logo", "extra"),
        ("symbol", "exe/logo", "old", "new"),
        ("repair", "exe/logo", "bad row"),
        ("repair", "exe/logo", "row.name"),
        ("repair", "exe/logo", "../function:name"),
        ("repair", "exe/logo", "exe/slus_004_22@function:name"),
        ("repair", "exe/logo", "unknown:name"),
        ("repair", "exe/logo", "function:name", "function:name"),
        ("repair", "exe/logo", "function:z_name", "data:a_name"),
        ("repair", "exe/logo", "function:path/name"),
        ("retained-lift", "exe/logo", "0x801CE758", "exact"),
        ("retained-lift", "exe/slus_004_22", SELECTOR, "exact"),
        ("retained-lift", "exe/logo", SELECTOR, "partial"),
        ("retained-lift", "exe/logo", SELECTOR, "exact", "bad row"),
        ("relocate-batch", "exe/logo", "ui"),
        ("relocate-batch", "exe/logo", "../ui", SELECTOR),
        ("relocate-batch", "exe/logo", "missing", SELECTOR),
        ("relocate-batch", "exe/logo", "ui", "0x801CE758"),
    ),
)
def test_invalid_routes_fail_before_any_body_read(tokens: tuple[str, ...]) -> None:
    with pytest.raises(ValueError):
        parse_cleanup_request(tokens)


def test_missing_unknown_or_ambiguous_selection_reads_zero_bodies() -> None:
    request = parse_cleanup_request(("docs", "docs/usage.md"))
    reads: list[Path] = []

    def read(path: Path) -> bytes:
        reads.append(path)
        return path.read_bytes()

    invalid = (
        replace(request, selected_skill=SelectedSkill("", "", ())),
        replace(
            request,
            selected_skill=SelectedSkill("unknown", ".pi/skills/unknown/SKILL.md", ()),
        ),
        replace(
            request,
            selected_skill=SelectedSkill(
                "repo-documentation-repair",
                ".pi/skills/repo-documentation-repair/SKILL.md",
                (
                    ".pi/skills/repo-documentation-repair/references/DOCUMENTATION_REPAIR.md",
                    ".pi/skills/bof3-naming-evidence/SKILL.md",
                ),
            ),
        ),
    )
    for candidate in invalid:
        with pytest.raises(ValueError, match="selected_skill"):
            cleanup_sections(ROOT, candidate, read)
    assert reads == []


def test_repair_rows_are_zero_or_more_and_identifiers_are_c_tokens() -> None:
    assert parse_cleanup_request(("repair", "exe/logo")).rows == ()
    for mode in ("symbol", "type"):
        request = parse_cleanup_request(
            (mode, "exe/logo", "old_name", "->", "new_name")
        )
        assert request.arguments == ("old_name", "new_name")
        for invalid in ("", "../old", "old/name", "old.name", "old-name", "9old"):
            with pytest.raises(ValueError, match="C identifier"):
                parse_cleanup_request((mode, "exe/logo", invalid, "->", "new_name"))


def test_docs_accept_only_existing_regular_explicit_markdown(tmp_path: Path) -> None:
    root = tmp_path
    (root / "docs").mkdir()
    (root / "docs/guide.md").write_text("guide\n")
    (root / "README.md").write_text("readme\n")
    (root / "AGENTS.md").write_text("agents\n")
    (root / "CHANGELOG.md").write_text("changes\n")
    outside = tmp_path.parent / "outside-cleanup.md"
    outside.write_text("outside\n")
    (root / "docs/link.md").symlink_to(outside)
    request = parse_cleanup_request(
        ("docs", "docs/guide.md", "README.md", "AGENTS.md"), root=root
    )
    assert request.arguments == ("docs/guide.md", "README.md", "AGENTS.md")
    for invalid in (
        "docs/link.md",
        "docs/missing.md",
        "../outside-cleanup.md",
        "CHANGELOG.md",
    ):
        with pytest.raises(ValueError, match="regular repository Markdown"):
            parse_cleanup_request(("docs", invalid), root=root)
    with pytest.raises(ValueError, match="canonical forward slashes"):
        parse_cleanup_request(("docs", "docs\\guide.md"), root=root)
    outside.unlink()


def test_full_frozen_request_is_revalidated_before_reads() -> None:
    request = parse_cleanup_request(("repair", "exe/logo", "function:func_801CE758"))
    reads: list[Path] = []
    forged = (
        replace(request, target="exe/missing"),
        replace(request, rows=("../function:name",)),
        replace(request, arguments=("function:other",)),
        replace(request, warning="forged"),
    )
    for candidate in forged:
        with pytest.raises((ValueError, IndexError)):
            cleanup_sections(ROOT, candidate, lambda path: reads.append(path) or b"")
    assert reads == []


def test_cleanup_agent_has_one_selected_body_read_and_no_mode_parser() -> None:
    text = (ROOT / ".pi/agents/bof3-cleanup.md").read_text()
    assert text.count("exactly one emitted `selected_skill.body` section") == 1
    assert "do not read that file again" in text
    assert "Never parse, normalize, infer, or switch modes" in text
    assert "inheritSkills: true" in text
