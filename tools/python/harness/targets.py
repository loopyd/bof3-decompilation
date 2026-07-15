from __future__ import annotations

from .binaries import (
    materialize_promoted_emi_targets,
    promote_entry,
    write_catalog,
)
from .domain import (
    TargetId,
    normalize_target_id,
    parse_address,
    parse_function_id,
)
from .domain.manifests import (
    TargetManifest,
    load_profiles,
    load_target_manifests,
)
from .domain.registry import ResolvedTarget

__all__ = [
    "TargetId",
    "normalize_target_id",
    "parse_address",
    "parse_function_id",
    "TargetManifest",
    "load_target_manifests",
    "load_profiles",
    "ResolvedTarget",
    "promote_entry",
    "write_catalog",
    "materialize_promoted_emi_targets",
]
