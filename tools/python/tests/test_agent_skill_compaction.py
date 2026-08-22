import json
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[3]
AUDIT = ROOT / ".pi/skills/agent-skill-compaction/scripts/audit.py"


@dataclass(frozen=True)
class CleanupRouteFixture:
    mode: str
    selected_skill: str
    references: tuple[str, ...]


def run(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        (sys.executable, str(AUDIT), *args),
        cwd=ROOT,
        text=True,
        capture_output=True,
    )


def skill_text(
    name: str,
    *,
    description: str = "Checks fixtures. Use when testing skill validation.",
    extra: str = "",
    body: str = "# Fixture\n",
) -> str:
    return f"---\nname: {name}\ndescription: {description}\n{extra}---\n\n{body}"


def write_skill(
    tmp_path: Path,
    *,
    directory: str = "fixture",
    name: str | None = None,
    description: str = "Checks fixtures. Use when testing skill validation.",
    extra: str = "",
    body: str = "# Fixture\n",
) -> Path:
    root = tmp_path / directory
    root.mkdir()
    (root / "SKILL.md").write_text(
        skill_text(name or directory, description=description, extra=extra, body=body)
    )
    return root


def strict_report(skill: Path) -> tuple[subprocess.CompletedProcess[str], dict]:
    result = run(str(skill), "--strict-skill-set", str(skill), "--check")
    return result, json.loads(result.stdout)


def test_compaction_audit_default_tree_and_self_baseline(tmp_path: Path) -> None:
    baseline = tmp_path / "baseline.json"
    assert run("--output", str(baseline)).returncode == 0
    result = run("--baseline", str(baseline), "--check")
    assert result.returncode == 0, result.stderr
    report = json.loads(result.stdout)
    assert report["totals"]["files"] >= 31
    assert not report["errors"]
    assert not report["strictFindings"]
    assert {finding["source"] for finding in report["strictFindings"]} <= {
        "agent-skills",
        "pi",
        "repository",
    }


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
    assert (
        run(
            ".pi/agents", ".pi/skills", "--baseline", str(baseline), "--check"
        ).returncode
        != 0
    )


@pytest.mark.parametrize(
    ("case", "directory", "name", "description", "extra", "body", "rule"),
    [
        ("directory mismatch", "fixture", "other", None, "", None, "directory-name"),
        ("uppercase name", "Fixture", "Fixture", None, "", None, "name"),
        ("leading hyphen", "-fixture", "-fixture", None, "", None, "name"),
        ("trailing hyphen", "fixture-", "fixture-", None, "", None, "name"),
        ("double hyphen", "fixture--name", "fixture--name", None, "", None, "name"),
        ("long name", "a" * 65, "a" * 65, None, "", None, "name"),
        (
            "missing what/when",
            "fixture",
            "fixture",
            "Checks fixtures.",
            "",
            None,
            "description-discovery",
        ),
        (
            "missing what",
            "fixture",
            "fixture",
            "Use when needed.",
            "",
            None,
            "description-discovery",
        ),
        (
            "no-op with trigger word",
            "fixture",
            "fixture",
            "Does things. Use when needed.",
            "",
            None,
            "description-discovery",
        ),
        (
            "capability without trigger clause",
            "fixture",
            "fixture",
            "Validates and repairs fixture files.",
            "",
            None,
            "description-discovery",
        ),
        (
            "unknown field",
            "fixture",
            "fixture",
            None,
            "argument-hint: TARGET\n",
            None,
            "unknown-field",
        ),
        ("long description", "fixture", "fixture", "x" * 1025, "", None, "description"),
        (
            "license type",
            "fixture",
            "fixture",
            None,
            "license: []\n",
            None,
            "field-license",
        ),
        (
            "compatibility type",
            "fixture",
            "fixture",
            None,
            "compatibility: []\n",
            None,
            "field-compatibility",
        ),
        (
            "long compatibility",
            "fixture",
            "fixture",
            None,
            f"compatibility: {'x' * 501}\n",
            None,
            "field-compatibility",
        ),
        (
            "metadata type",
            "fixture",
            "fixture",
            None,
            "metadata: []\n",
            None,
            "field-metadata",
        ),
        (
            "metadata value",
            "fixture",
            "fixture",
            None,
            "metadata:\n  count: 1\n",
            None,
            "field-metadata",
        ),
        (
            "allowed tools type",
            "fixture",
            "fixture",
            None,
            "allowed-tools: []\n",
            None,
            "field-allowed-tools",
        ),
        (
            "pi field type",
            "fixture",
            "fixture",
            None,
            "disable-model-invocation: nope\n",
            None,
            "field-disable-model-invocation",
        ),
        (
            "body budget",
            "fixture",
            "fixture",
            None,
            "",
            "x" * 19996,
            "body-token-budget",
        ),
        (
            "broken link",
            "fixture",
            "fixture",
            None,
            "",
            "# Fixture\n[missing](references/MISSING.md)\n",
            "markdown-structure",
        ),
        (
            "absolute link",
            "fixture",
            "fixture",
            None,
            "",
            "# Fixture\n[absolute](/tmp/file.md)\n",
            "relative-link",
        ),
        (
            "file URI",
            "fixture",
            "fixture",
            None,
            "",
            "# Fixture\n[absolute](file:///tmp/file.md)\n",
            "relative-link",
        ),
        (
            "unbalanced fence",
            "fixture",
            "fixture",
            None,
            "",
            "# Fixture\n```text\n",
            "markdown-structure",
        ),
        (
            "invalid closing fence suffix",
            "fixture",
            "fixture",
            None,
            "",
            "# Fixture\n```text\nvalue\n``` trailing\n",
            "markdown-structure",
        ),
    ],
)
def test_strict_skill_validation_rejects_invalid_classes(
    tmp_path: Path,
    case: str,
    directory: str,
    name: str,
    description: str | None,
    extra: str,
    body: str | None,
    rule: str,
) -> None:
    skill = write_skill(
        tmp_path,
        directory=directory,
        name=name,
        description=description
        or "Checks fixtures. Use when testing skill validation.",
        extra=extra,
        body=body or "# Fixture\n",
    )
    result, report = strict_report(skill)
    assert result.returncode != 0, case
    assert rule in {finding["rule"] for finding in report["strictFindings"]}, case


@pytest.mark.parametrize(
    ("directory", "description", "extra", "body"),
    [
        (
            "a" * 64,
            "x" * 984 + " Validates fixtures. Use when needed.",
            "compatibility: " + "x" * 500 + "\n",
            "x" * 19995,
        ),
        (
            "fixture",
            "Checks fixtures. Use when testing.",
            "license: MIT\nallowed-tools: read grep\nmetadata:\n  owner: BOF3\ndisable-model-invocation: true\n",
            "# Fixture\n",
        ),
        (
            "fixture",
            "Checks fixtures. Use when testing.",
            'license: ""\nallowed-tools: ""\ncompatibility: ""\nmetadata: {}\n',
            "# Fixture\n",
        ),
        (
            "fixture",
            "Checks fixtures. Use when testing.",
            "license: x\nallowed-tools: x\ncompatibility: x\n",
            "# Fixture\n",
        ),
        (
            "fixture",
            "Checks fixtures. Use when testing.",
            f"license: {'x' * 1000}\nallowed-tools: {'x' * 1000}\n",
            "# Fixture\n",
        ),
    ],
)
def test_strict_skill_validation_accepts_boundaries(
    tmp_path: Path, directory: str, description: str, extra: str, body: str
) -> None:
    skill = write_skill(
        tmp_path, directory=directory, description=description, extra=extra, body=body
    )
    result, report = strict_report(skill)
    assert result.returncode == 0, result.stderr
    assert not report["strictFindings"]


@pytest.mark.parametrize(
    "description",
    [
        "Compacts and organizes project skill Markdown. Use after any skill edit.",
        "Lift or review one target-qualified BOF3 function. Use for BOF3 matching tasks.",
        "Audits naming evidence and validates receipts. Invoke when preparing a naming-audit/v3 report.",
        "Repairs scoped repository documentation. Select this skill when docs disagree with implementation.",
    ],
)
def test_strict_skill_accepts_project_discovery_descriptions(
    tmp_path: Path, description: str
) -> None:
    skill = write_skill(tmp_path, description=description)
    result, report = strict_report(skill)
    assert result.returncode == 0, result.stderr
    assert not report["strictFindings"]


def test_markdown_ignores_fenced_links_and_references(tmp_path: Path) -> None:
    skill = write_skill(
        tmp_path,
        body=(
            "# Fixture\n"
            "```markdown\n"
            "[missing](references/MISSING.md)\n"
            "[missing-ref][no-definition]\n"
            "```\n"
        ),
    )
    result, report = strict_report(skill)
    assert result.returncode == 0, result.stderr
    assert not report["strictFindings"]


def test_token_estimator_uses_unicode_code_points_at_exact_boundary(
    tmp_path: Path,
) -> None:
    below = write_skill(tmp_path, directory="below", body="é" * 19995)
    result, report = strict_report(below)
    assert result.returncode == 0, result.stderr
    assert report["strictPolicy"]["tokenEstimator"] == (
        "ceil(body Unicode code points / 4)"
    )

    boundary = write_skill(tmp_path, directory="boundary", body="é" * 19996)
    result, report = strict_report(boundary)
    assert result.returncode != 0
    assert "body-token-budget" in {
        finding["rule"] for finding in report["strictFindings"]
    }


def test_strict_skill_requires_direct_links_to_every_reference(tmp_path: Path) -> None:
    skill = write_skill(tmp_path, body="# Fixture\n[First](references/FIRST.md)\n")
    references = skill / "references"
    references.mkdir()
    (references / "FIRST.md").write_text("# First\n[Second](SECOND.md)\n")
    (references / "SECOND.md").write_text("# Second\n")

    result, report = strict_report(skill)
    assert result.returncode != 0
    findings = [
        finding
        for finding in report["strictFindings"]
        if finding["rule"] == "direct-reference"
    ]
    assert len(findings) == 1
    assert findings[0]["message"].endswith("references/SECOND.md")

    (skill / "SKILL.md").write_text(
        skill_text(
            "fixture",
            body="# Fixture\n[First](references/FIRST.md)\n[Second](references/SECOND.md)\n",
        )
    )
    result, report = strict_report(skill)
    assert result.returncode == 0, result.stderr
    assert not report["strictFindings"]


def test_phase5_cleanup_router_fixtures_capture_routes_refs_and_bytes(
    tmp_path: Path,
) -> None:
    fixtures = (
        CleanupRouteFixture(
            "symbol", "bof3-identity-maintenance", ("IDENTITY_TRANSACTIONS.md",)
        ),
        CleanupRouteFixture(
            "docs", "repo-documentation-repair", ("DOCUMENTATION_REPAIR.md",)
        ),
        CleanupRouteFixture(
            "audit-target", "bof3-naming-evidence", ("NAMING_AUDIT_V3.md",)
        ),
    )
    loaded_bytes: dict[str, int] = {}
    for fixture in fixtures:
        skill = tmp_path / fixture.selected_skill
        references = skill / "references"
        references.mkdir(parents=True)
        body = f"# {fixture.selected_skill}\n".encode()
        (skill / "SKILL.md").write_bytes(body)
        for name in fixture.references:
            (references / name).write_text(f"# {name}\n")
        loaded_bytes[fixture.mode] = len(body) + sum(
            (references / name).stat().st_size for name in fixture.references
        )

    assert {fixture.mode: fixture.selected_skill for fixture in fixtures} == {
        "symbol": "bof3-identity-maintenance",
        "docs": "repo-documentation-repair",
        "audit-target": "bof3-naming-evidence",
    }
    assert all(loaded_bytes[fixture.mode] > 0 for fixture in fixtures)


def test_strict_skill_set_is_atomic_and_narrow(tmp_path: Path) -> None:
    valid = write_skill(tmp_path, directory="valid")
    invalid = write_skill(tmp_path, directory="invalid", name="wrong")
    unrelated = write_skill(tmp_path, directory="unrelated", name="also-wrong")

    result = run(
        str(unrelated), "--strict-skill-set", str(valid), str(invalid), "--check"
    )
    report = json.loads(result.stdout)
    assert result.returncode != 0
    paths = {finding["path"] for finding in report["strictFindings"]}
    assert str(invalid / "SKILL.md") in paths
    assert str(unrelated / "SKILL.md") not in paths
    assert len(report["strictSkillSet"]) == 2
    assert {row["path"] for row in report["files"]} == {
        str(valid / "SKILL.md"),
        str(invalid / "SKILL.md"),
    }
    assert not report["errors"]


def test_strict_skill_rejects_malformed_or_missing_frontmatter(tmp_path: Path) -> None:
    for content in ("# Missing\n", "---\nname: fixture\ndescription: [\n"):
        skill = tmp_path / f"case-{len(list(tmp_path.iterdir()))}"
        skill.mkdir()
        (skill / "SKILL.md").write_text(content)
        result = run("--strict-skill-set", str(skill), "--check")
        report = json.loads(result.stdout)
        assert result.returncode != 0
        assert "front-matter" in {
            finding["rule"] for finding in report["strictFindings"]
        }


def markdown_links(path: Path) -> tuple[str, ...]:
    return tuple(re.findall(r"\[[^]]+\]\(([^)]+)\)", path.read_text()))


def markdown_headings(path: Path) -> tuple[str, ...]:
    return tuple(re.findall(r"^#+\s+(.+)$", path.read_text(), re.MULTILINE))


def test_phase6_semantic_owners_are_direct_and_non_dangling() -> None:
    naming = ROOT / ".pi/skills/bof3-naming-evidence"
    identity = ROOT / ".pi/skills/bof3-identity-maintenance"
    owners = {
        naming / "SKILL.md": {
            "references/NAMING_AUDIT_V3.md",
            "../bof3-identity-maintenance/references/IDENTITY_TRANSACTIONS.md",
            "../bof3-identity-maintenance/references/BYTE_SAFE_COSMETICS.md",
        },
        identity / "SKILL.md": {
            "references/IDENTITY_TRANSACTIONS.md",
            "references/SOURCE_RELOCATION.md",
            "references/BYTE_SAFE_COSMETICS.md",
        },
        naming / "references/NAMING_AUDIT_V3.md": {
            "../../bof3-identity-maintenance/references/IDENTITY_TRANSACTIONS.md#authority-ceiling",
            "../../bof3-identity-maintenance/references/BYTE_SAFE_COSMETICS.md#spelling-transaction-rung",
        },
        identity / "references/IDENTITY_TRANSACTIONS.md": {
            "BYTE_SAFE_COSMETICS.md#metadata-preflight-and-authority",
            "BYTE_SAFE_COSMETICS.md#naming-and-validation",
            "SOURCE_RELOCATION.md#ownership-and-invariants",
        },
        identity / "references/BYTE_SAFE_COSMETICS.md": {
            "../../bof3-naming-evidence/references/NAMING_AUDIT_V3.md",
            "IDENTITY_TRANSACTIONS.md",
            "SOURCE_RELOCATION.md",
            "#spelling-transaction-rung",
            "../../../../include/base/types.h",
            "../../../../docs/agents/lessons.md",
        },
        identity / "references/SOURCE_RELOCATION.md": {
            "../../bof3-naming-evidence/references/NAMING_AUDIT_V3.md#recursive-inventory-and-audit-authority",
        },
    }
    for source, expected in owners.items():
        links = set(markdown_links(source))
        assert expected <= links
        for link in links:
            if link.startswith("#"):
                anchor = link[1:]
                target = source
            else:
                relative, _, anchor = link.partition("#")
                target = (source.parent / relative).resolve()
                assert target.is_file(), (source, link)
            if anchor:
                assert re.search(
                    rf"^#+\s+{re.escape(anchor.replace('-', ' '))}\s*$",
                    target.read_text(),
                    re.MULTILINE | re.IGNORECASE,
                ), (source, link)

    naming_headings = set(markdown_headings(naming / "references/NAMING_AUDIT_V3.md"))
    assert {
        "Evidence gate",
        "Recursive inventory and audit authority",
        "Audit evidence",
    } <= naming_headings
    audit_text = (naming / "references/NAMING_AUDIT_V3.md").read_text()
    for field in (
        "target count",
        "header count",
        "target paths",
        "header paths",
        "resolved target identities",
        "resolved header identities",
    ):
        assert audit_text.count(field) == 1, field
    for field in (
        "`path`",
        "`contract`",
        "`evidence`",
        "`smallest repair`",
        "`validation`",
        "`human approval`",
    ):
        assert audit_text.count(field) == 1, field
    identity_headings = set(
        markdown_headings(identity / "references/IDENTITY_TRANSACTIONS.md")
    )
    assert {
        "Parent phase order and classification",
        "Transaction",
        "Repair and readiness",
    } <= identity_headings
    cosmetic_headings = set(
        markdown_headings(identity / "references/BYTE_SAFE_COSMETICS.md")
    )
    assert {
        "Metadata preflight and authority",
        "Safe ladder",
        "Guarded ladder",
        "Never safe as cleanup",
        "Naming and validation",
    } <= cosmetic_headings


def test_phase6_skill_set_structure_triggers_links_and_bytes() -> None:
    skills = {
        "bof3-naming-evidence": ("NAMING_AUDIT_V3.md",),
        "bof3-identity-maintenance": (
            "IDENTITY_TRANSACTIONS.md",
            "SOURCE_RELOCATION.md",
            "BYTE_SAFE_COSMETICS.md",
        ),
        "repo-documentation-repair": ("DOCUMENTATION_REPAIR.md",),
    }
    paths = [ROOT / ".pi/skills" / name for name in skills]
    result = run(
        "--strict-skill-set",
        *(str(path) for path in paths),
        str(ROOT / ".pi/skills/bof3-re"),
        "--check",
    )
    report = json.loads(result.stdout)
    assert result.returncode == 0, result.stderr
    assert not report["strictFindings"]

    descriptions = []
    for name, references in skills.items():
        root = ROOT / ".pi/skills" / name
        body = (root / "SKILL.md").read_text()
        descriptions.append(body.split("description: ", 1)[1].splitlines()[0])
        assert tuple(
            sorted(path.name for path in (root / "references").glob("*.md"))
        ) == tuple(sorted(references))
        assert all(f"references/{reference}" in body for reference in references)
        assert (root / "SKILL.md").stat().st_size + sum(
            (root / "references" / reference).stat().st_size for reference in references
        ) < 16_000
    assert len(set(descriptions)) == len(skills)

    pipeline = ROOT / ".pi/skills/bof3-re/references/PIPELINE_VALIDATION.md"
    for consumer in (
        ROOT / ".pi/skills/bof3-re/SKILL.md",
        ROOT / ".pi/agents/bof3-reverse.md",
        ROOT / ".pi/agents/bof3-review.md",
    ):
        assert "PIPELINE_VALIDATION.md" in consumer.read_text()
    assert pipeline.is_file()
    legacy_cleanup = ROOT / ".pi/skills/bof3-re/references" / "CLEANUP"
    assert not legacy_cleanup.exists()
