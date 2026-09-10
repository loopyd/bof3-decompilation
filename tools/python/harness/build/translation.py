"""Adapt one complete maspsx translation to metadata-owned function sections."""

from __future__ import annotations

from pathlib import Path
import re

from harness.common.lexicon import iter_c_lexemes
from harness.domain.functions import parse_function_records
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.domain.sources import compiled_symbol_name
from harness.domain.tags import count_function_metadata

from .sections import partition_assembly


def prepare_translation(root: Path, source: Path, assembly: str) -> str:
    """Leave ordinary units intact; partition only completely owned grouped lifts.

    This does not select a compiler, authorize consolidation, or prove matching.
    The caller retains its configured compiler and full translation-unit flags.
    """

    source = source.resolve()
    text = source.read_text(encoding="utf-8")
    if count_function_metadata(text) < 2:
        return assembly
    if any(
        lexeme.group().startswith(("/*", "//"))
        and re.search(r"@(compiler|gcc|cflags|flags)\b", lexeme.group())
        for lexeme in iter_c_lexemes(text)
    ):
        raise ValueError(
            "per-function compiler annotations are not implemented; "
            "use reviewed compatible object profiles or defer consolidation"
        )
    records = parse_function_records(text, validate_progress=False)
    relative = source.relative_to(root.resolve()).as_posix()
    owners = [
        manifest
        for manifest in load_target_manifests(root).values()
        if relative in manifest.sources
    ]
    if len(owners) != 1:
        raise ValueError("grouped compilation requires exactly one explicit lift owner")
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
    return partition_assembly(assembly, names)
