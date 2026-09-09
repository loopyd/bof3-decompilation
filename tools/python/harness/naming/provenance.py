"""Validate canonical exact-conclusion provenance metadata."""

from __future__ import annotations

import re
from pathlib import PurePosixPath
from typing import Any

from harness.naming.capabilities import (
    PRODUCTION_EXACT_CAPABILITIES,
    ExactCapabilityRegistry,
)

_SHA256 = re.compile(r"[0-9a-f]{64}").fullmatch
PROVENANCE_FIELD = "conclusion_provenance"


def capability_eligible_rows(
    target: str,
    *,
    registry: ExactCapabilityRegistry = PRODUCTION_EXACT_CAPABILITIES,
) -> set[str]:
    """Return the code-owned terminal-row selectors enabled for one target."""
    return {
        row_key
        for (row_key, _selector), entry in registry.items()
        if entry.consumer.target == target
    }


def _canonical_path(value: object, *, absolute: bool, label: str) -> str:
    if not isinstance(value, str) or not value or "\\" in value:
        raise ValueError(f"{label} is not a canonical path")
    path = PurePosixPath(value)
    if (
        path.is_absolute() != absolute
        or value != path.as_posix()
        or value in {".", "/"}
    ):
        raise ValueError(f"{label} is not a canonical path")
    if any(part in {"", ".", ".."} for part in path.parts):
        raise ValueError(f"{label} is not a canonical path")
    return value


def require_canonical_provenance(row: dict[str, Any], selector: str) -> dict[str, Any]:
    """Validate the one closed, currently supported terminal provenance schema."""
    provenance = row.get(PROVENANCE_FIELD)
    keys = {
        "kind",
        "evidence",
        "evidence_sha256",
        "initializer_report_sha256",
        "evidence_namespace",
    }
    if not isinstance(provenance, dict) or set(provenance) != keys:
        raise ValueError(f"{selector} conclusion provenance has a non-canonical shape")
    if provenance.get("kind") != "exact-capability":
        raise ValueError(f"{selector} conclusion provenance kind is unsupported")
    for key in ("evidence_sha256", "initializer_report_sha256"):
        value = provenance.get(key)
        if not isinstance(value, str) or _SHA256(value) is None:
            raise ValueError(
                f"{selector} conclusion provenance requires lowercase SHA-256 ({key})"
            )
    namespace = provenance.get("evidence_namespace")
    if not isinstance(namespace, dict) or set(namespace) != {
        "mode",
        "root",
        "namespace",
    }:
        raise ValueError(f"{selector} conclusion provenance namespace is non-canonical")
    mode = namespace.get("mode")
    if mode not in {"default", "explicit"}:
        raise ValueError(f"{selector} conclusion provenance namespace mode is invalid")
    absolute = mode == "explicit"
    root = _canonical_path(
        namespace.get("root"), absolute=absolute, label="namespace root"
    )
    namespace_path = _canonical_path(
        namespace.get("namespace"), absolute=absolute, label="namespace path"
    )
    evidence = _canonical_path(
        provenance.get("evidence"), absolute=absolute, label="evidence path"
    )
    root_path = PurePosixPath(root)
    namespace_value = PurePosixPath(namespace_path)
    evidence_value = PurePosixPath(evidence)
    if (
        namespace_value.parent != root_path
        or evidence_value.parent.parent != namespace_value
    ):
        raise ValueError(f"{selector} conclusion provenance escapes its namespace")
    return provenance
