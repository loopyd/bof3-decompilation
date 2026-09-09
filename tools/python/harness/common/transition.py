"""Exact reviewed sequential private transitions for fresh check-only execution."""

from __future__ import annotations

from pathlib import Path

from harness.common import execution as context
from harness.common.history import validate_reviewed_history
from harness.common.workspace import workspace_state


def validate_transition(
    root: Path,
    original: dict,
    expected: str,
    intervening: list,
    owner: str,
    *,
    manifest_validator,
) -> None:
    if not intervening:
        raise ValueError("reviewed private transition requires intervening pins")
    applications = validate_sequence(
        root,
        original,
        expected,
        intervening,
        owner,
        manifest_validator=manifest_validator,
    )
    last = applications[-1]
    validate_endpoint(
        applications,
        last,
        context.capture(
            root,
            last["manifest"],
            last["review_context"]["initial_state"].get("participating_targets"),
        ),
        context.build_state(root),
        workspace_state(root),
    )


def validate_sequence(
    root, original, expected, intervening, owner, *, manifest_validator
):
    if not isinstance(intervening, list):
        raise ValueError("reviewed private transition requires intervening pins")
    envelopes = [(original, expected)]
    for pin in intervening:
        if not isinstance(pin, dict) or set(pin) != {
            "envelope",
            "expected_envelope_digest",
        }:
            raise ValueError("invalid intervening private pin")
        envelopes.append((pin["envelope"], pin["expected_envelope_digest"]))
    if len({pin for _, pin in envelopes}) != len(envelopes):
        raise ValueError("repeated private transition")
    for envelope, pin in envelopes:
        validate_reviewed_history(root, envelope, pin, owner, manifest_validator)
    applications = [envelope["application"] for envelope, _ in envelopes]
    protected = set(applications[0]["manifest"]["allowed_paths"])
    for application in applications[1:]:
        if protected & set(application["changed_paths"]):
            raise ValueError(
                "intervening private edit overlaps original owned post-state"
            )
    for before, after in zip(applications, applications[1:]):
        _state_equal(
            before,
            before["review_context"]["final_state"],
            after,
            after["review_context"]["initial_state"],
        )
        if (
            before["review_context"]["final_build"]
            != after["review_context"]["initial_build"]
        ):
            raise ValueError("unreviewed build gap between private transactions")
        _workspace_equal(before, after["manifest"]["workspace_baseline"]["state"])
    return applications


def validate_endpoint(applications, endpoint, state, build, workspace):
    last = applications[-1]
    _state_equal(last, last["review_context"]["final_state"], endpoint, state)
    if last["review_context"]["final_build"] != build:
        raise ValueError("unreviewed current build drift")
    _workspace_equal(last, workspace)
    for name in applications[0]["manifest"]["allowed_paths"]:
        if (
            state["inputs"].get(name)
            != applications[0]["review_context"]["final_state"]["inputs"][name]
        ):
            raise ValueError("original owned post-state drifted")


def _state_equal(before: dict, left: dict, after: dict, right: dict) -> None:
    # Manifest evidence has different paths per transaction; history validation
    # checks each retained byte/mode independently, never excludes native inputs.
    evidence = context._evidence_paths(before["manifest"]) | context._evidence_paths(
        after["manifest"]
    )

    differing_paths = set(left["inputs"]) ^ set(right["inputs"])
    if not differing_paths <= evidence:
        raise ValueError("unreviewed private input membership gap")

    def projection(state):
        return {
            **state,
            "inputs": {
                p: v for p, v in state["inputs"].items() if p not in differing_paths
            },
        }

    if projection(left) != projection(right):
        raise ValueError("unreviewed private input transition gap")


def _workspace_equal(application: dict, current: dict) -> None:
    baseline = application["manifest"]["workspace_baseline"]["state"]
    changed = set(application["changed_paths"])
    if {p: v for p, v in baseline.items() if p not in changed} != {
        p: v for p, v in current.items() if p not in changed
    }:
        raise ValueError("unreviewed private workspace transition gap")
    for path in changed:
        if (
            path in current
            and current[path]["sha256"] != application["post_state"][path]
        ):
            raise ValueError("private workspace post-state mismatch")
