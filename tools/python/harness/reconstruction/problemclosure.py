"""The shared closed-Problem policy for one approved Problem bundle.

The order is the policy.  The index is admitted **content-free** first
(``upstream == []``, ``role == problem``, ``origin == approved``, the reviewed
bundle schema, the active run, exactly one primary document, the reviewed primary
schema), then the caller resolves the sealed Problem document the index already
names, then the nested edges are closed, and only then may a caller run the
generic transitive traversal.  A malformed ``upstream`` or a nested reference
leaving the Problem is therefore refused before that traversal can read
forbidden content.

Predicates raise PreparationRefused before reading content outside the Problem.
"""

from __future__ import annotations

from typing import Literal
from harness.common.failures import FailureEvent

from harness.reconstruction import bundles, contract

PROBLEM_ROLE = "problem"
PROBLEM_ORIGIN = "approved"


def require(condition: object, message: str) -> None:
    contract.require(condition, message)


def require_closed_index(index: dict, run_name: str) -> dict:
    """Admit the frozen Problem index content-free and return its primary entry."""

    require(
        index["upstream"] == [],
        "the frozen Problem index must declare no upstream",
    )
    require(index["role"] == PROBLEM_ROLE, "the active bundle is not a Problem")
    require(
        index["origin"] == PROBLEM_ORIGIN,
        "the Problem origin is not approved",
    )
    require(
        index["schema"] == contract.BUNDLE_SCHEMA,
        "the Problem bundle schema is not the reviewed one",
    )
    require(
        index["run"] == run_name,
        "the Problem does not belong to the active run",
    )
    require(
        len(index["documents"]) == 1,
        "the sealed Problem must publish exactly one primary document",
    )
    primary = index["documents"][0]
    require(
        primary["schema"] == contract.PROBLEM_SCHEMA,
        "the primary document is not a Problem",
    )
    return primary


def require_closed_document(document: dict) -> None:
    """The caller-resolved primary document must still be the reviewed Problem."""

    require(
        document.get("schema") == contract.PROBLEM_SCHEMA,
        "the resolved document is not a Problem",
    )


def require_closed_inventory(index: dict, document: dict) -> None:
    """The index must carry exactly the bound images and rules, nothing else."""

    referenced = _referenced_files(document)
    indexed = {}
    for entry in index["files"]:
        require(
            entry["path"] not in indexed,
            f"the Problem index repeats a file path: {entry['path']}",
        )
        indexed[entry["path"]] = (entry["sha256"], entry["bytes"])
    require(
        set(indexed) == set(referenced),
        "the Problem index inventory is not exactly the bound images and rules",
    )
    for path, declared in indexed.items():
        require(
            declared == referenced[path],
            f"the indexed file {path} differs from its bound reference",
        )


def require_closed_references(
    run_dir,
    document: dict,
    *,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
) -> None:
    """Apply the closed Problem policy, then resolve every admitted nested edge."""

    bound = _referenced_files(document)
    for kind, ref in bundles.iter_references(document):
        if kind == "doc":
            require(
                ref.get("expected_bundle") is None and ref["bundle"] == PROBLEM_ROLE,
                "a Problem document reference must stay inside the Problem",
            )
            bundles.resolve_document(
                run_dir, ref, owner=PROBLEM_ROLE, io_mode=io_mode, events=events
            )
        else:
            _closed_file(ref, bound, "a Problem file reference")
            bundles.resolve_file(
                run_dir, ref, owner=PROBLEM_ROLE, io_mode=io_mode, events=events
            )


def _referenced_files(document: dict) -> dict:
    """The exact ``path -> (sha256, bytes)`` set the Problem document binds."""

    refs = {}
    for image in document["images"]:
        refs[image["file"]["path"]] = (image["file"]["sha256"], image["file"]["bytes"])
    source = document["rules"]["source"]
    refs[source["path"]] = (source["sha256"], source["bytes"])
    return refs


def _closed_file(ref: dict, bound: dict, subject: str) -> None:
    """A Problem file reference must name one bound image or the bound rules file."""

    require(ref.get("expected_bundle") is None, f"{subject} must not pin a bundle")
    require(ref["bundle"] == PROBLEM_ROLE, f"{subject} must stay inside the Problem")
    require(
        bound.get(ref["path"]) == (ref["sha256"], ref["bytes"]),
        f"{subject} is not a bound Problem image or rules file",
    )


__all__ = [
    "PROBLEM_ORIGIN",
    "PROBLEM_ROLE",
    "require_closed_document",
    "require_closed_index",
    "require_closed_inventory",
    "require_closed_references",
]
