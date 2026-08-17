"""Focused tests for the strict metadata-backed source registry.

Covers: @source/@behavior as the sole lift identity (no filename parsing or
fallback), deterministic malformed-candidate and duplicate-address errors,
safe helper-file classification, map/Splat-agreed compiled symbol identity,
and the migrated consumers (lift m2c, analyzer, asm-diff, decomp-status
preflight, rev-query mission, permute, layout).
"""

from __future__ import annotations

import struct
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

import pytest

from harness.analysis.engine import EngineIdentity, build_snapshot
from harness.commands import _common, permute
from harness.domain import parse_function_id, parse_progress_tags, resolve_function
from harness.domain.sources import (
    CompiledSymbolError,
    LiftMetadataError,
    SourceAddressCollision,
    collect_source_addresses,
    compiled_symbol_name,
    expected_lift_sources,
    lift_metadata,
    owning_manifest,
    resolve_source_for_address,
    reviewed_function_name,
    source_address,
)
from harness.domain.layout import parse_splat_layout
from harness.match._asm_diff_payload import AsmDiffRequest
from harness.match._asm_diff_run import _asm_diff_resolve
from harness.match._asm_resolve import infer_size_from_sibling_sources

TARGET_TOML = (
    'schema = "harness.target/v2"\n'
    'id = "exe/logo"\n'
    'kind = "executable"\n'
    'source_dir = "src/exe/logo"\n'
    'binary = "out/binaries/exe/logo.bin"\n'
    'splat = "config/targets/exe/logo/splat.yaml"\n'
    "load_address = 0x801CE000\n"
)


def _target(
    root: Path, *, sources: tuple[str, ...] = ("src/exe/logo/initSelectionState.c",)
) -> None:
    target = root / "config" / "targets" / "exe" / "logo" / "target.toml"
    target.parent.mkdir(parents=True)
    lines = [
        'schema = "harness.target/v2"',
        'id = "exe/logo"',
        'kind = "executable"',
        'source_dir = "src/exe/logo"',
        'binary = "out/binaries/exe/logo.bin"',
        'splat = "config/targets/exe/logo/splat.yaml"',
        "load_address = 0x801CE000",
        "sources = [" + ", ".join(f'"{s}"' for s in sources) + "]",
    ]
    target.write_text("\n".join(lines) + "\n", encoding="utf-8")
    for claimed in sources:
        claimed_path = root / claimed
        claimed_path.parent.mkdir(parents=True, exist_ok=True)
        if not claimed_path.exists():
            claimed_path.write_text("void placeholder(void) {}\n", encoding="utf-8")


def _map(root: Path, text: str) -> None:
    symbols = root / "config" / "targets" / "exe" / "logo" / "symbols.txt"
    symbols.parent.mkdir(parents=True, exist_ok=True)
    symbols.write_text(text, encoding="utf-8")


def _splat(
    root: Path, name: str = "initSelectionState", address: int = 0x801CE758
) -> None:
    splat = root / "config" / "targets" / "exe" / "logo" / "splat.yaml"
    splat.parent.mkdir(parents=True, exist_ok=True)
    splat.write_text(
        "name: logo\n"
        "options:\n"
        "  create_c_files: false\n"
        "  platform: psx\n"
        "segments:\n"
        "- name: main\n"
        "  type: code\n"
        "  start: 0\n"
        "  vram: 0x801CE000\n"
        "  subsegments:\n"
        f"  - - {address - 0x801CE000}\n"
        "    - c\n"
        f"    - {name}\n",
        encoding="utf-8",
    )


def _layout(root: Path) -> SimpleNamespace:
    return SimpleNamespace(root=root, build_dir=root / "build", out_dir=root / "out")


# -- strict metadata identity --------------------------------------------------


def test_partial_progress_metadata_is_parsable() -> None:
    text = """/* @status partial
 * @match 58.33
 * @residual allocator mismatch remains.
 */"""
    assert parse_progress_tags(text) == (
        "partial",
        58.33,
        "allocator mismatch remains.",
    )


def test_partial_progress_metadata_is_atomic() -> None:
    with pytest.raises(ValueError, match="requires @status, @match, and @residual"):
        parse_progress_tags("/* @status partial */")


def test_source_address_from_tag_only(tmp_path: Path) -> None:
    renamed = tmp_path / "initSelectionState.c"
    renamed.write_text("/* @source 0x80100004 @behavior stages selection */\n")
    assert source_address(renamed) == 0x80100004


def test_source_address_never_parses_filename(tmp_path: Path) -> None:
    raw = tmp_path / "func_80100000.c"
    raw.write_text("void f(void) {}\n")
    with pytest.raises(LiftMetadataError, match="missing_source"):
        source_address(raw)


def test_source_address_missing_raises(tmp_path: Path) -> None:
    orphan = tmp_path / "initState.c"
    orphan.write_text("void f(void) {}\n")
    with pytest.raises(LiftMetadataError, match="missing_source"):
        source_address(orphan)


def test_lift_metadata_requires_behavior(tmp_path: Path) -> None:
    partial = tmp_path / "initState.c"
    partial.write_text("/* @source 0x80100000 */\n", encoding="utf-8")
    with pytest.raises(LiftMetadataError, match="missing_behavior"):
        lift_metadata(partial)


def test_lift_metadata_requires_source(tmp_path: Path) -> None:
    partial = tmp_path / "helper.c"
    partial.write_text("/* @behavior helper prose */\n", encoding="utf-8")
    with pytest.raises(LiftMetadataError, match="missing_source"):
        lift_metadata(partial)


# -- collection, helpers, collisions -------------------------------------------


def test_collect_source_addresses_semantic_and_raw(tmp_path: Path) -> None:
    (tmp_path / "func_80100004.c").write_text(
        "/* @source 0x80100004 @behavior x */\n", encoding="utf-8"
    )
    (tmp_path / "initState.c").write_text(
        "/* @source 0x80100000 @behavior x */\n", encoding="utf-8"
    )
    rows = collect_source_addresses(tmp_path)
    assert [(row[0].name, row[1]) for row in rows] == [
        ("initState.c", 0x80100000),
        ("func_80100004.c", 0x80100004),
    ]


def test_collect_nested_source_uses_target_relative_path(tmp_path: Path) -> None:
    nested = tmp_path / "battle" / "dispatchState.c"
    nested.parent.mkdir()
    nested.write_text(
        "/* @source 0x80100008 @behavior dispatches battle state */\n",
        encoding="utf-8",
    )
    rows = collect_source_addresses(
        tmp_path, expected_lifts={"battle/dispatchState": 0x80100008}
    )
    assert rows == [(nested, 0x80100008)]
    assert resolve_source_for_address(tmp_path, 0x80100008) == nested


def test_collect_skips_helper_files_without_identity(tmp_path: Path) -> None:
    (tmp_path / "symbols.c").write_text("WEAK_SYMBOL_AT(x, 0x80100000);\n")
    (tmp_path / "helper.c").write_text(
        "/* @behavior authored helper prose */\n", encoding="utf-8"
    )
    (tmp_path / "lift.c").write_text(
        "/* @source 0x80100008 @behavior x */\n", encoding="utf-8"
    )
    rows = collect_source_addresses(tmp_path)
    assert [row[0].name for row in rows] == ["lift.c"]


def test_collect_ignores_legacy_stem_without_tags(tmp_path: Path) -> None:
    """A func_<ADDR> filename alone never confers lift candidacy."""
    (tmp_path / "func_80100000.c").write_text("void f(void) {}\n", encoding="utf-8")
    assert collect_source_addresses(tmp_path) == []


def test_collect_expected_func_stem_requires_tag_not_filename(
    tmp_path: Path,
) -> None:
    """Splat-expected candidacy is satisfied by @source, never the stem."""
    (tmp_path / "func_80100000.c").write_text("void f(void) {}\n", encoding="utf-8")
    with pytest.raises(LiftMetadataError, match="missing_source"):
        collect_source_addresses(tmp_path, expected_lifts={"func_80100000": 0x80100000})


def test_collect_flags_expected_lift_without_tags(tmp_path: Path) -> None:
    (tmp_path / "initState.c").write_text("void f(void) {}\n", encoding="utf-8")
    with pytest.raises(LiftMetadataError, match="missing_source"):
        collect_source_addresses(tmp_path, expected_lifts={"initState": 0x80100000})


def test_collect_flags_expected_address_mismatch(tmp_path: Path) -> None:
    (tmp_path / "initState.c").write_text(
        "/* @source 0x80100004 @behavior x */\n", encoding="utf-8"
    )
    with pytest.raises(LiftMetadataError, match="address_mismatch"):
        collect_source_addresses(tmp_path, expected_lifts={"initState": 0x80100000})


def test_collect_flags_duplicate_addresses(tmp_path: Path) -> None:
    (tmp_path / "func_80100000.c").write_text(
        "/* @source 0x80100000 @behavior x */\n", encoding="utf-8"
    )
    (tmp_path / "dup.c").write_text(
        "/* @source 0x80100000 @behavior x */\n", encoding="utf-8"
    )
    with pytest.raises(SourceAddressCollision, match="collision 0x80100000"):
        collect_source_addresses(tmp_path)


def test_resolve_source_for_address(tmp_path: Path) -> None:
    (tmp_path / "func_80100000.c").write_text(
        "/* @source 0x80100000 @behavior x */\n", encoding="utf-8"
    )
    (tmp_path / "initState.c").write_text(
        "/* @source 0x80100004 @behavior x */\n", encoding="utf-8"
    )
    renamed = resolve_source_for_address(tmp_path, 0x80100004)
    raw = resolve_source_for_address(tmp_path, 0x80100000)
    assert renamed is not None and renamed.name == "initState.c"
    assert raw is not None and raw.name == "func_80100000.c"
    assert resolve_source_for_address(tmp_path, 0x80100008) is None


# -- expected lift sources from Splat ------------------------------------------


def test_expected_lift_sources_nested_source_path(tmp_path: Path) -> None:
    _target(tmp_path)
    splat = tmp_path / "config" / "targets" / "exe" / "logo" / "splat.yaml"
    splat.parent.mkdir(parents=True, exist_ok=True)
    splat.write_text(
        "name: logo\nsegments:\n- name: main\n  type: code\n  start: 0\n"
        "  vram: 0x801CE000\n  subsegments:\n"
        "  - - 0x758\n    - c\n    - dispatchState\n"
        "    - '@source: src/exe/logo/battle/dispatchState.c'\n",
        encoding="utf-8",
    )
    layout = parse_splat_layout(splat, 0x801CE000)
    assert expected_lift_sources(layout, tmp_path / "src" / "exe" / "logo") == {
        "battle/dispatchState": 0x801CE758
    }


def test_expected_lift_sources_list_and_dict_forms(tmp_path: Path) -> None:
    _target(tmp_path)
    splat = tmp_path / "config" / "targets" / "exe" / "logo" / "splat.yaml"
    splat.parent.mkdir(parents=True, exist_ok=True)
    splat.write_text(
        "name: logo\n"
        "options:\n"
        "  create_c_files: false\n"
        "  platform: psx\n"
        "segments:\n"
        "- name: main\n"
        "  type: code\n"
        "  start: 0\n"
        "  vram: 0x801CE000\n"
        "  subsegments:\n"
        "  - - 0x758\n"
        "    - c\n"
        "    - initSelectionState\n"
        "    - '@source: src/exe/logo/initSelectionState.c'\n"
        "    - '@behavior: stages selection'\n"
        "  - - 0x790\n"
        "    - c\n"
        "    - name: dispatchState\n"
        "      source: src/exe/logo/dispatchState.c\n"
        "      behavior: dispatches state\n"
        "  - - 0x7C0\n"
        "    - asm\n"
        "    - func_801CE7C0\n",
        encoding="utf-8",
    )
    layout = parse_splat_layout(splat, 0x801CE000)
    expected = expected_lift_sources(layout, tmp_path / "src" / "exe" / "logo")
    assert expected == {
        "initSelectionState": 0x801CE758,
        "dispatchState": 0x801CE790,
    }


# -- compiled symbol identity (map/Splat agreement) -----------------------------


def test_reviewed_function_name_semantic(tmp_path: Path) -> None:
    _target(tmp_path)
    _map(tmp_path, "initSelectionState = 0x801CE758;\n")
    _splat(tmp_path)
    layout = parse_splat_layout(
        tmp_path / "config/targets/exe/logo/splat.yaml", 0x801CE000
    )
    assert (
        reviewed_function_name(tmp_path, "exe/logo", 0x801CE758, layout=layout)
        == "initSelectionState"
    )


def test_reviewed_function_name_accepts_map_owned_lift_inside_bin(
    tmp_path: Path,
) -> None:
    _target(tmp_path)
    _map(tmp_path, "initSelectionState = 0x801CE758;\n")
    splat = tmp_path / "config/targets/exe/logo/splat.yaml"
    splat.write_text(
        "name: logo\nsegments:\n- [0, bin, image]\n- [4096]\n",
        encoding="utf-8",
    )
    layout = parse_splat_layout(splat, 0x801CE000)
    assert (
        reviewed_function_name(tmp_path, "exe/logo", 0x801CE758, layout=layout)
        == "initSelectionState"
    )


def test_reviewed_function_name_raw(tmp_path: Path) -> None:
    _target(tmp_path)
    _map(tmp_path, "func_801CE758 = 0x801CE758;\n")
    _splat(tmp_path, name="func_801CE758")
    layout = parse_splat_layout(
        tmp_path / "config/targets/exe/logo/splat.yaml", 0x801CE000
    )
    assert (
        reviewed_function_name(tmp_path, "exe/logo", 0x801CE758, layout=layout)
        == "func_801CE758"
    )


def test_reviewed_function_name_missing_map_entry_never_synthesized(
    tmp_path: Path,
) -> None:
    _target(tmp_path)
    _splat(tmp_path)
    layout = parse_splat_layout(
        tmp_path / "config/targets/exe/logo/splat.yaml", 0x801CE000
    )
    with pytest.raises(CompiledSymbolError, match="no target-local map entry"):
        reviewed_function_name(tmp_path, "exe/logo", 0x801CE758, layout=layout)


def test_reviewed_function_name_rejects_data_symbol(tmp_path: Path) -> None:
    _target(tmp_path)
    _map(tmp_path, "D_801CE758 = 0x801CE758;\n")
    _splat(tmp_path)
    layout = parse_splat_layout(
        tmp_path / "config/targets/exe/logo/splat.yaml", 0x801CE000
    )
    with pytest.raises(CompiledSymbolError, match="data symbol"):
        reviewed_function_name(tmp_path, "exe/logo", 0x801CE758, layout=layout)


def test_reviewed_function_name_rejects_boundary_disagreement(tmp_path: Path) -> None:
    _target(tmp_path)
    _map(tmp_path, "initSelectionState = 0x801CE758;\n")
    _splat(tmp_path, name="otherName")
    layout = parse_splat_layout(
        tmp_path / "config/targets/exe/logo/splat.yaml", 0x801CE000
    )
    with pytest.raises(CompiledSymbolError, match="disagrees"):
        reviewed_function_name(tmp_path, "exe/logo", 0x801CE758, layout=layout)


def test_reviewed_function_name_shared_map_never_used(tmp_path: Path) -> None:
    """A shared-map-only entry is not target-owned compiled identity."""
    _target(tmp_path)
    shared = tmp_path / "config/targets/shared/symbols.txt"
    shared.parent.mkdir(parents=True)
    shared.write_text("initSelectionState = 0x801CE758;\n", encoding="utf-8")
    _splat(tmp_path)
    layout = parse_splat_layout(
        tmp_path / "config/targets/exe/logo/splat.yaml", 0x801CE000
    )
    with pytest.raises(CompiledSymbolError, match="no target-local map entry"):
        reviewed_function_name(tmp_path, "exe/logo", 0x801CE758, layout=layout)


def test_compiled_symbol_name_source_path(tmp_path: Path) -> None:
    _target(tmp_path)
    _map(tmp_path, "initSelectionState = 0x801CE758;\n")
    _splat(tmp_path)
    source_dir = tmp_path / "src" / "exe" / "logo"
    source_dir.mkdir(parents=True, exist_ok=True)
    source = source_dir / "initSelectionState.c"
    source.write_text("/* @source 0x801CE758 @behavior x */\n", encoding="utf-8")
    assert compiled_symbol_name(tmp_path, source, 0x801CE758) == "initSelectionState"


def test_compiled_symbol_name_nested_source_path(tmp_path: Path) -> None:
    _target(tmp_path, sources=("src/exe/logo/runtime/initSelectionState.c",))
    _map(tmp_path, "initSelectionState = 0x801CE758;\n")
    _splat(tmp_path)
    source = tmp_path / "src" / "exe" / "logo" / "runtime" / "initSelectionState.c"
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text("/* @source 0x801CE758 @behavior x */\n", encoding="utf-8")
    assert compiled_symbol_name(tmp_path, source, 0x801CE758) == "initSelectionState"


def test_owning_manifest_claimed_root_and_nested_sources(tmp_path: Path) -> None:
    _target(
        tmp_path,
        sources=(
            "src/exe/logo/initState.c",
            "src/exe/logo/runtime/nested.c",
        ),
    )
    root_source = tmp_path / "src" / "exe" / "logo" / "initState.c"
    root_source.parent.mkdir(parents=True, exist_ok=True)
    root_source.write_text("/* @source 0x80100000 @behavior x */\n")
    nested = tmp_path / "src" / "exe" / "logo" / "runtime" / "nested.c"
    nested.parent.mkdir(parents=True, exist_ok=True)
    nested.write_text("/* @source 0x80100004 @behavior x */\n")
    unrelated = tmp_path / "src" / "other" / "helper.c"
    unrelated.parent.mkdir(parents=True)
    unrelated.write_text("/* @behavior helper */\n")

    assert owning_manifest(tmp_path, root_source).id.value == "exe/logo"  # type: ignore[union-attr]
    assert owning_manifest(tmp_path, nested).id.value == "exe/logo"  # type: ignore[union-attr]
    assert owning_manifest(tmp_path, unrelated) is None


def test_owning_manifest_unclaimed_nested_source_is_none(tmp_path: Path) -> None:
    """Ownership comes only from explicit claims, never source_dir ancestry."""
    _target(tmp_path)
    unclaimed = tmp_path / "src" / "exe" / "logo" / "runtime" / "deep.c"
    unclaimed.parent.mkdir(parents=True)
    unclaimed.write_text("/* @source 0x80100008 @behavior x */\n")

    assert owning_manifest(tmp_path, unclaimed) is None


def test_infer_size_from_sibling_sources_uses_manifest_root(
    tmp_path: Path,
) -> None:
    _target(
        tmp_path,
        sources=(
            "src/exe/logo/runtime/first.c",
            "src/exe/logo/ui/second.c",
        ),
    )
    source_dir = tmp_path / "src" / "exe" / "logo"
    first = source_dir / "runtime" / "first.c"
    second = source_dir / "ui" / "second.c"
    first.parent.mkdir(parents=True, exist_ok=True)
    second.parent.mkdir(parents=True, exist_ok=True)
    first.write_text("/* @source 0x80100000 @behavior x */\n")
    second.write_text("/* @source 0x80100010 @behavior x */\n")

    size = infer_size_from_sibling_sources(first, 0x80100000, root=tmp_path)
    assert size == 0x10  # next-higher lift in another subsystem folder
    assert (
        infer_size_from_sibling_sources(first, 0x80100000) is None
    )  # folder-local fallback sees no sibling


def test_asm_diff_resolve_nested_source_output_owner(tmp_path: Path) -> None:
    _target(tmp_path, sources=("src/exe/logo/runtime/initSelectionState.c",))
    _map(tmp_path, "initSelectionState = 0x801CE758;\n")
    _splat(tmp_path)
    binary = tmp_path / "out" / "binaries" / "exe" / "logo.bin"
    binary.parent.mkdir(parents=True)
    payload = bytearray(0x800)
    struct.pack_into("<I", payload, 0x758, 0x03E00008)  # jr $ra at 0x801CE758
    binary.write_bytes(payload)

    source = tmp_path / "src" / "exe" / "logo" / "runtime" / "initSelectionState.c"
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text("/* @source 0x801CE758 @behavior x */\n", encoding="utf-8")

    request = AsmDiffRequest(
        source_path=source,
        address=0x801CE758,
        binary_path=binary,
        load_address=0x801CE000,
        diagnostics=False,
    )
    resolved = _asm_diff_resolve(_layout(tmp_path), request)  # type: ignore[arg-type]

    assert resolved["address"] == 0x801CE758
    assert resolved["function_name"] == "initSelectionState"
    assert resolved["original_size"] == 8
    assert resolved["output_dir"].name == "initSelectionState"
    assert resolved["output_dir"].parent.name == "exe_logo"  # manifest-owned slug


# -- registry resolved-function record ---------------------------------------


def test_registry_resolve_function_semantic_name(tmp_path: Path) -> None:
    _target(tmp_path, sources=("src/exe/logo/renamed.c",))
    _map(tmp_path, "initSelectionState = 0x801CE758;\n")
    _splat(tmp_path)
    source_dir = tmp_path / "src" / "exe" / "logo"
    source_dir.mkdir(parents=True, exist_ok=True)
    renamed = source_dir / "renamed.c"
    renamed.write_text("/* @source 0x801CE758 @behavior x */\n", encoding="utf-8")

    resolved = resolve_function(tmp_path, "exe/logo@0x801CE758")

    assert resolved.id.address == 0x801CE758
    assert resolved.manifest.id.value == "exe/logo"
    assert resolved.source == renamed
    assert resolved.compiled_symbol == "initSelectionState"


def test_registry_resolve_function_accepts_parsed_id(tmp_path: Path) -> None:
    _target(tmp_path)
    _map(tmp_path, "initSelectionState = 0x801CE758;\n")
    _splat(tmp_path)

    resolved = resolve_function(tmp_path, parse_function_id("exe/logo@0x801CE758"))

    assert resolved.id.address == 0x801CE758
    assert resolved.compiled_symbol == "initSelectionState"


def test_registry_resolve_function_raw_name_and_missing_source(tmp_path: Path) -> None:
    _target(tmp_path)
    _map(tmp_path, "func_801CE758 = 0x801CE758;\n")
    _splat(tmp_path, name="func_801CE758")

    resolved = resolve_function(tmp_path, "exe/logo@0x801CE758")

    assert resolved.compiled_symbol == "func_801CE758"
    assert resolved.source is None  # never fabricated from the address


def test_registry_resolve_function_unmapped_symbol_is_none(tmp_path: Path) -> None:
    _target(tmp_path)
    _splat(tmp_path)

    resolved = resolve_function(tmp_path, "exe/logo@0x801CE758")

    assert resolved.compiled_symbol is None
    assert resolved.source is None


def test_registry_resolve_function_unknown_target_raises(tmp_path: Path) -> None:
    with pytest.raises(ValueError, match="unknown target"):
        resolve_function(tmp_path, "exe/unknown@0x801CE758")


def test_registry_resolve_function_rejects_misplaced_manifest(
    tmp_path: Path,
) -> None:
    """A manifest outside its canonical path must not fabricate identity.

    ``config/targets/exe/wrong/target.toml`` declaring ``id = exe/logo`` is
    discovered by the manifest loader but must fail the same canonical
    path/identity validation as :func:`resolve_target`, never return a
    fabricated nonexistent ``manifest_path``.
    """

    claimed = tmp_path / "src" / "exe" / "logo" / "initSelectionState.c"
    claimed.parent.mkdir(parents=True, exist_ok=True)
    claimed.write_text("/* @source 0x801CE758 @behavior x */\n", encoding="utf-8")
    wrong = tmp_path / "config" / "targets" / "exe" / "wrong" / "target.toml"
    wrong.parent.mkdir(parents=True)
    wrong.write_text(
        'schema = "harness.target/v2"\n'
        'id = "exe/logo"\n'
        'kind = "executable"\n'
        'source_dir = "src/exe/logo"\n'
        'binary = "out/binaries/exe/logo.bin"\n'
        'splat = "config/targets/exe/logo/splat.yaml"\n'
        "load_address = 0x801CE000\n"
        'sources = ["src/exe/logo/initSelectionState.c"]\n',
        encoding="utf-8",
    )

    with pytest.raises(FileNotFoundError, match="target manifest missing"):
        resolve_function(tmp_path, "exe/logo@0x801CE758")


# -- migrated consumers --------------------------------------------------------


def test_lift_m2c_resolve_function_renamed_source(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _target(tmp_path)
    source_dir = tmp_path / "src" / "exe" / "logo"
    source_dir.mkdir(parents=True, exist_ok=True)
    renamed = source_dir / "initSelectionState.c"
    renamed.write_text("/* @source 0x801CE758 @behavior x */\n", encoding="utf-8")
    monkeypatch.setattr(_common, "repo_layout", lambda: _layout(tmp_path))

    function, manifest, source = _common.resolve_function_selector(
        "exe/logo@0x801CE758"
    )

    assert function.address == 0x801CE758
    assert manifest.id.value == "exe/logo"
    assert source == renamed


def test_lift_m2c_resolve_function_missing_source_is_none(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _target(tmp_path)
    (tmp_path / "src" / "exe" / "logo").mkdir(parents=True, exist_ok=True)
    monkeypatch.setattr(_common, "repo_layout", lambda: _layout(tmp_path))

    _function, _manifest, source = _common.resolve_function_selector(
        "exe/logo@0x801CE758"
    )

    assert source is None  # never a fabricated func_<ADDR> path


def test_analyzer_snapshot_is_lifted_for_renamed_source(tmp_path: Path) -> None:
    binary = tmp_path / "target.bin"
    binary.write_bytes(b"\0" * 64)
    engine = EngineIdentity("rizin", tmp_path / "rizin", "test", {})
    functions = [{"offset": 0x80100000, "size": 16, "name": "func_80100000"}]
    source_dir = tmp_path / "src"
    source_dir.mkdir()
    renamed = source_dir / "initState.c"
    renamed.write_text("/* @source 0x80100000 @behavior x */\n", encoding="utf-8")

    with patch("harness.analysis.engine._run_analysis", return_value=(functions, [])):
        snapshot = build_snapshot(
            engine, binary, 0x80100000, "test", source_dir=source_dir
        )

    function = snapshot.functions[0]
    assert function.is_lifted
    assert function.source == str(renamed)


def test_analyzer_snapshot_ignores_helper_files(tmp_path: Path) -> None:
    binary = tmp_path / "target.bin"
    binary.write_bytes(b"\0" * 64)
    engine = EngineIdentity("rizin", tmp_path / "rizin", "test", {})
    functions = [{"offset": 0x80100000, "size": 16, "name": "func_80100000"}]
    source_dir = tmp_path / "src"
    source_dir.mkdir()
    (source_dir / "symbols.c").write_text("WEAK_SYMBOL_AT(x, 0x80100000);\n")

    with patch("harness.analysis.engine._run_analysis", return_value=(functions, [])):
        snapshot = build_snapshot(
            engine, binary, 0x80100000, "test", source_dir=source_dir
        )

    assert not snapshot.functions[0].is_lifted
    assert snapshot.functions[0].source is None


def test_analyzer_detects_source_address_collision(tmp_path: Path) -> None:
    binary = tmp_path / "target.bin"
    binary.write_bytes(b"\0" * 64)
    engine = EngineIdentity("rizin", tmp_path / "rizin", "test", {})
    functions = [{"offset": 0x80100000, "size": 16, "name": "func_80100000"}]
    source_dir = tmp_path / "src"
    source_dir.mkdir()
    (source_dir / "func_80100000.c").write_text(
        "/* @source 0x80100000 @behavior x */\n", encoding="utf-8"
    )
    (source_dir / "dup.c").write_text(
        "/* @source 0x80100000 @behavior x */\n", encoding="utf-8"
    )

    with patch("harness.analysis.engine._run_analysis", return_value=(functions, [])):
        with pytest.raises(SourceAddressCollision, match="collision 0x80100000"):
            build_snapshot(engine, binary, 0x80100000, "test", source_dir=source_dir)


def test_asm_diff_resolve_symbol_identity_map_backed(tmp_path: Path) -> None:
    _target(tmp_path)
    _map(tmp_path, "initSelectionState = 0x801CE758;\n")
    _splat(tmp_path)
    binary = tmp_path / "out" / "binaries" / "exe" / "logo.bin"
    binary.parent.mkdir(parents=True)
    payload = bytearray(0x800)
    struct.pack_into("<I", payload, 0x758, 0x03E00008)  # jr $ra at 0x801CE758
    binary.write_bytes(payload)

    source_dir = tmp_path / "src" / "exe" / "logo"
    source_dir.mkdir(parents=True, exist_ok=True)
    source = source_dir / "initSelectionState.c"
    source.write_text("/* @source 0x801CE758 @behavior x */\n", encoding="utf-8")

    request = AsmDiffRequest(
        source_path=source,
        address=0x801CE758,
        binary_path=binary,
        load_address=0x801CE000,
        diagnostics=False,
    )
    resolved = _asm_diff_resolve(_layout(tmp_path), request)  # type: ignore[arg-type]

    assert resolved["address"] == 0x801CE758
    assert resolved["function_name"] == "initSelectionState"
    assert resolved["original_size"] == 8


def test_permute_function_name_map_backed(tmp_path: Path) -> None:
    _target(tmp_path)
    _map(tmp_path, "initSelectionState = 0x801CE758;\n")
    _splat(tmp_path)
    source = tmp_path / "src" / "exe" / "logo" / "initSelectionState.c"
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text("/* @source 0x801CE758 @behavior x */\n", encoding="utf-8")

    assert permute.resolve_function_name(source, None, tmp_path) == "initSelectionState"
    assert permute.resolve_function_name(source, "explicit", tmp_path) == "explicit"


def test_permute_function_name_no_map_entry_raises(tmp_path: Path) -> None:
    _target(tmp_path)
    _splat(tmp_path)
    source = tmp_path / "src" / "exe" / "logo" / "initSelectionState.c"
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text("/* @source 0x801CE758 @behavior x */\n", encoding="utf-8")

    with pytest.raises(CompiledSymbolError, match="no target-local map entry"):
        permute.resolve_function_name(source, None, tmp_path)
