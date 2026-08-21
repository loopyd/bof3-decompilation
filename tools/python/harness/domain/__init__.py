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

from .tags import (
    BEHAVIOR_TAG_RE,
    MATCH_TAG_RE,
    PREFIXED_RAW_NAME_RE,
    RAW_SYMBOL_NAME_RE,
    RESIDUAL_TAG_RE,
    SOURCE_TAG_RE,
    STATUS_TAG_RE,
    lift_lifecycle,
    parse_behavior_tag,
    parse_declaration_kind_tag,
    parse_declaration_source_tag,
    parse_progress_tags,
    parse_source_tag,
)

_SOURCE_EXPORTS = {
    "CompiledSymbolError",
    "LiftMetadataError",
    "SourceAddressCollision",
    "compiled_symbol_name",
    "expected_lift_sources",
    "lift_metadata",
    "local_include_files",
    "reviewed_function_name",
    "source_address",
}
_REGISTRY_EXPORTS = {
    "ResolvedFunction",
    "ResolvedTarget",
    "lookup_target_manifest",
    "resolve_all_targets",
    "resolve_function",
    "resolve_target",
}


def __getattr__(name: str):
    """Load analyzer-backed registry exports only when a caller needs them."""

    if name in _REGISTRY_EXPORTS:
        from . import registry

        return getattr(registry, name)
    if name in _SOURCE_EXPORTS:
        from . import sources

        return getattr(sources, name)
    raise AttributeError(name)


__all__ = [
    "BEHAVIOR_TAG_RE",
    "CompanionAbi",
    "CompiledSymbolError",
    "FUNCTION_ID_FORMAT",
    "MATCH_TAG_RE",
    "FUNCTION_ID_HELP",
    "CompanionOverlay",
    "CompanionStaticCall",
    "FunctionId",
    "LiftMetadataError",
    "lift_lifecycle",
    "PREFIXED_RAW_NAME_RE",
    "RAW_SYMBOL_NAME_RE",
    "RESIDUAL_TAG_RE",
    "ResolvedFunction",
    "ResolvedTarget",
    "SOURCE_TAG_RE",
    "STATUS_TAG_RE",
    "SourceAddressCollision",
    "TargetId",
    "TargetManifest",
    "compiled_symbol_name",
    "expected_lift_sources",
    "lift_metadata",
    "local_include_files",
    "load_target_manifests",
    "lookup_target_manifest",
    "normalize_target_id",
    "parse_address",
    "parse_behavior_tag",
    "parse_declaration_kind_tag",
    "parse_declaration_source_tag",
    "parse_function_id",
    "parse_progress_tags",
    "parse_source_tag",
    "resolve_all_targets",
    "resolve_function",
    "resolve_target",
    "reviewed_function_name",
    "source_address",
]
