"""Read-only macro candidate inspection for evidence-backed resolution."""

from __future__ import annotations

from pathlib import Path
from typing import Any

from harness.analysis.index import connect
from harness.common.digests import digest
from harness.macros.blocks import block_opportunities_payload
from harness.macros.impact import definition_impact_payload
from harness.macros.opportunities import macro_opportunities_payload
from harness.macros.queries import macro_uses_payload, macros_payload
from harness.macros.review import GUARDS, candidate_concerns, candidate_observations
from harness.macros.similarity import near_duplicates_payload

SCHEMA = "bof3.macro-resolution/v1"


def _member_target(member: dict[str, Any]) -> str:
    return member.get("target") or member.get("function", "").rsplit("@", 1)[0]


def describe_candidate(
    root: Path,
    candidate_id: str,
    *,
    target: str | None = None,
    min_instructions: int | None = None,
) -> dict[str, Any]:
    """Resolve a global candidate ID to current evidence, never approval."""
    connection = connect(root)
    try:
        if (
            target is not None
            and connection.execute(
                "SELECT 1 FROM targets WHERE id = ?", (target,)
            ).fetchone()
            is None
        ):
            raise ValueError(f"unknown macro resolution target: {target}")
        if candidate_id.startswith("assembly_block:"):
            candidates = block_opportunities_payload(
                connection, root, min_instructions=min_instructions
            )
        elif candidate_id.startswith("near_duplicate:"):
            candidates = near_duplicates_payload(connection, root, target=None, limit=0)
        else:
            candidates = macro_opportunities_payload(
                connection, root, target=None, kind=None, limit=0
            )
        matches = [row for row in candidates if row["id"] == candidate_id]
        if len(matches) != 1:
            raise ValueError(f"macro candidate not found uniquely: {candidate_id}")
        candidate = matches[0]
        members = candidate["members"]
        selected = [
            member
            for member in members
            if target is None or _member_target(member) == target
        ]
        if not selected:
            raise ValueError(f"macro candidate has no members in target: {target}")
        sources = []
        for member in selected:
            if member.get("function"):
                row = connection.execute(
                    "SELECT source FROM functions WHERE id = ?", (member["function"],)
                ).fetchone()
                source = row[0] if row else None
            else:
                source = member.get("source_path")
            if source:
                path = (root / source).resolve()
                if not path.is_relative_to(root.resolve()) or not path.is_file():
                    raise ValueError(f"stale macro member source: {source}")
                source = path.relative_to(root.resolve()).as_posix()
            sources.append({"member": member, "source_path": source})
        source_owners = {
            (_member_target(row["member"]), row["source_path"]) for row in sources
        }
        uses = [
            use
            for use in macro_uses_payload(
                connection, target=target, pattern=None, limit=0
            )
            if (use["target_id"], use["source_path"]) in source_owners
        ]
        definition_ids = {use["definition_id"] for use in uses}
        definitions = [
            definition
            for definition in macros_payload(
                connection, target=target, pattern=None, classification=None, limit=0
            )
            if definition["id"] in definition_ids
        ]
        impact = definition_impact_payload(connection, definition_ids)
    finally:
        connection.close()
    concerns = candidate_concerns(candidate["kind"])
    return {
        "schema": SCHEMA,
        "candidate": candidate,
        "candidate_fingerprint": digest(candidate),
        "view_target": target,
        "members": sources,
        "external_members": [member for member in members if member not in selected],
        "existing_macro_uses": uses,
        "existing_definitions": definitions,
        "definition_impact": impact,
        "macro_use_scope": "member_source_files; not occurrence or expansion binding",
        "binding_evidence": "lexical_name_matches_only; includes and conditional selection unproven",
        "resolution": {
            "status": "blocked",
            "safe_application_count": 0,
            "supported_concerns": concerns,
            "required_semantic_guards": sorted(GUARDS),
            "required_observations": sorted(candidate_observations(candidate["kind"])),
            "next_steps": [
                "Inspect member source and original bytes; establish useful equivalent C shape.",
                "Resolve all semantic guards and parameter mapping; inspect all affected use sites.",
                "Review global definition impact, including other candidates and parameter values; target focus is not write authority.",
                "If the abstraction already exists, use prepare-existing/check-existing/review-existing/verify-existing; never manufacture edits or count existing work as an application.",
                "Obtain independent review of explicit owners and their current fingerprints.",
                *(
                    [
                        "Shared templates require an independently pinned exact private wrapper per declared target, with at least two targets."
                    ]
                    if "shared_template" in concerns
                    else []
                ),
                "Use macro-audit prepare/run, then independent review/final-verify with externally pinned digests.",
            ],
        },
    }
