"""Adapt one complete maspsx translation to metadata-owned function sections."""

from __future__ import annotations

from pathlib import Path

from harness.build.arguments import read_source
from harness.build.dispatch import Dispatch, validate_dispatch
from harness.build.expansion import expand_function_members
from harness.domain.functions import parse_function_records, requires_function_expansion
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.domain.sources import compiled_symbol_name, owning_manifest
from harness.domain.tags import count_function_metadata, parse_source_tag

from .sections import partition_assembly


def prepare_translation(
    root: Path, source: Path, assembly: str, *, dispatch: Dispatch | None = None
) -> str:
    """Leave ordinary units intact; partition only completely owned grouped lifts.

    This does not select a compiler, authorize consolidation, or prove matching.
    The caller retains its configured compiler and full translation-unit flags.
    """

    root = root.resolve()
    source = source.resolve()
    if dispatch is not None:
        if (
            dispatch.source is None
            or dispatch.source.resolve() != source
            or dispatch.root != root
        ):
            raise ValueError("translation source differs from its compiler invocation")
        validate_dispatch(dispatch)
    text = read_source(source).replace("\r\n", "\n").replace("\r", "\n")
    count = count_function_metadata(text)
    if count < 2:
        if count == 1:
            manifest = owning_manifest(root, source)
            if manifest is not None and requires_function_expansion(text):
                if dispatch is None:
                    raise ValueError(
                        "translation expansion requires an active configured compiler invocation"
                    )
                expand_function_members(root, source)
                validate_dispatch(dispatch)
                name = compiled_symbol_name(
                    root,
                    source,
                    parse_source_tag(text),
                    manifest=manifest,
                )
                partition_assembly(assembly, [name])
        if dispatch is not None:
            validate_dispatch(dispatch)
        return assembly
    if dispatch is None:
        raise ValueError(
            "grouped translation requires an active configured compiler invocation"
        )
    validate_dispatch(dispatch)
    records = parse_function_records(text, validate_progress=False)
    if requires_function_expansion(text):
        expand_function_members(root, source)
        validate_dispatch(dispatch)
    relative = source.relative_to(root.resolve()).as_posix()
    owners = [
        manifest
        for manifest in load_target_manifests(root).values()
        if relative in manifest.sources + manifest.support_sources
    ]
    if len(owners) != 1:
        raise ValueError(
            "grouped compilation requires exactly one explicit source owner"
        )
    manifest = owners[0]
    layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    names = []
    for record in records:
        name = compiled_symbol_name(
            root, source, record.address, manifest=manifest, layout=layout
        )
        if record.kind == "function" and record.spelling != name:
            raise ValueError(
                "function spelling disagrees with reviewed compiled symbol"
            )
        names.append(name)
    result = partition_assembly(assembly, names)
    validate_dispatch(dispatch)
    return result
