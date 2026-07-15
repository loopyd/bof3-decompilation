"""Stable domain identifiers used by the harness workflows."""

from .ids import (
    FunctionId,
    TargetId,
    normalize_target_id,
    parse_address,
    parse_function_id,
)
from .manifests import (
    Component,
    Profile,
    TargetManifest,
    load_components,
    load_profiles,
    load_target_manifests,
)
from .registry import (
    ResolvedTarget,
    resolve_all_targets,
    resolve_target,
)

__all__ = [
    "FunctionId",
    "Component",
    "Profile",
    "ResolvedTarget",
    "TargetId",
    "TargetManifest",
    "load_profiles",
    "load_components",
    "load_target_manifests",
    "normalize_target_id",
    "parse_address",
    "parse_function_id",
    "resolve_all_targets",
    "resolve_target",
]
