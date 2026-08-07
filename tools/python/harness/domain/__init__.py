"""Stable domain identifiers used by the harness workflows."""

from .ids import (
    FUNCTION_ID_FORMAT,
    FUNCTION_ID_HELP,
    FunctionId,
    TargetId,
    normalize_target_id,
    parse_address,
    parse_function_id,
)
from .manifests import (
    CompanionAbi,
    CompanionOverlay,
    CompanionStaticCall,
    TargetManifest,
    load_target_manifests,
)
from .registry import (
    ResolvedTarget,
    lookup_target_manifest,
    resolve_all_targets,
    resolve_target,
)
from .tags import (
    BEHAVIOR_TAG_RE,
    PREFIXED_RAW_NAME_RE,
    RAW_SYMBOL_NAME_RE,
    SOURCE_TAG_RE,
    parse_behavior_tag,
    parse_declaration_source_tag,
    parse_source_tag,
)

__all__ = [
    "BEHAVIOR_TAG_RE",
    "CompanionAbi",
    "FUNCTION_ID_FORMAT",
    "FUNCTION_ID_HELP",
    "CompanionOverlay",
    "CompanionStaticCall",
    "FunctionId",
    "PREFIXED_RAW_NAME_RE",
    "RAW_SYMBOL_NAME_RE",
    "ResolvedTarget",
    "SOURCE_TAG_RE",
    "TargetId",
    "TargetManifest",
    "load_target_manifests",
    "lookup_target_manifest",
    "normalize_target_id",
    "parse_address",
    "parse_behavior_tag",
    "parse_declaration_source_tag",
    "parse_function_id",
    "parse_source_tag",
    "resolve_all_targets",
    "resolve_target",
]
