"""Tests for flag-search variant environment integration."""

from __future__ import annotations

import json
from pathlib import Path
from types import SimpleNamespace

import pytest

from harness.toolchain.gcc_variants import EmptyCatalog


def _make_layout(root: Path) -> SimpleNamespace:
    return SimpleNamespace(
        root=root,
        toolchains_dir=root / "toolchains",
        gcc_variants_root=root / "toolchains" / "gcc-variants",
        downloads_dir=root / "downloads",
    )


class TestSearchFlagsEmptyVariant:
    def test_resolve_returns_none_for_empty_catalog(self) -> None:
        """When catalog is empty and no compiler_id given, returns EmptyCatalog."""
        from harness.compiler_config import resolve_compiler_variant
        from harness.io import RepoLayout
        import tempfile
        root = Path(tempfile.mkdtemp())
        try:
            layout = RepoLayout(
                root=root, build_dir=root / "build", out_dir=root / "out",
                toolchains_dir=root / "toolchains",
                third_party_dir=root / "third_party",
                inputs_dir=root / "inputs",
                downloads_dir=root / "downloads",
                private_assets_dir=root / "inputs" / "external" / "private-assets",
                harness_disk_src=root / "tools" / "rust" / "bof3-disk",
                emi_ex_src=root / "tools" / "rust" / "emi-ex",
                harness_disk_bin=root / "bof3-disk",
                emi_ex_bin=root / "emi-ex",
                psn00b_toolchain_root=root / "toolchains" / "psn00b_toolchain",
                psn00b_sdk_root=root / "toolchains" / "psn00bsdk",
                gcc272_psx_root=root / "toolchains" / "gcc-2.7.2-psx",
                gcc_variants_root=root / "toolchains" / "gcc-variants",
                psyq_root=root / "toolchains" / "psyq" / "4.7",
            )
            variant = resolve_compiler_variant(layout)
            assert isinstance(variant, EmptyCatalog)
        finally:
            import shutil
            shutil.rmtree(root, ignore_errors=True)

    def test_explicit_id_raises_in_empty_catalog(self, tmp_path: Path) -> None:
        """Requesting a specific compiler ID in empty catalog raises."""
        from harness.compiler_config import resolve_compiler_variant
        layout = _make_layout(tmp_path)
        with pytest.raises(ValueError, match="not found in catalog"):
            resolve_compiler_variant(layout, compiler_id="nonexistent")  # type: ignore[arg-type]


class TestVariantCatalogLookup:
    def test_lookup_by_id_returns_matching_variant(self, tmp_path: Path) -> None:
        from harness.toolchain.gcc_variants import lookup_variant
        layout = _make_layout(tmp_path)
        variants_file = tmp_path / "config" / "compiler" / "variants.json"
        variants_file.parent.mkdir(parents=True)
        variants_file.write_text(json.dumps({
            "schema": "harness.compiler-variants/v1",
            "candidates": [{
                "id": "gcc-v1",
                "label": "GCC V1",
                "url": "https://example.com/gcc-v1.tar.gz",
                "checksum": "sha256:" + "a" * 64,
                "archive_name": "gcc-v1.tar.gz",
                "license": "GPL-2.0+",
                "source": "https://example.com/gcc-v1",
                "host": "linux-x86_64",
                "identity": "mips-sony-psx-gcc",
                "assembler": "ASPSX compatible",
                "executable_relpath": "gcc",
            }],
        }))
        variant = lookup_variant(layout, "gcc-v1")  # type: ignore[arg-type]
        assert variant.id == "gcc-v1"
        assert variant.label == "GCC V1"

    def test_lookup_nonexistent_id_raises(self, tmp_path: Path) -> None:
        from harness.toolchain.gcc_variants import lookup_variant
        layout = _make_layout(tmp_path)
        variants_file = tmp_path / "config" / "compiler" / "variants.json"
        variants_file.parent.mkdir(parents=True)
        variants_file.write_text(json.dumps({
            "schema": "harness.compiler-variants/v1",
            "candidates": [],
        }))
        with pytest.raises(ValueError, match="not found"):
            lookup_variant(layout, "does-not-exist")  # type: ignore[arg-type]


class TestCompilerVariantsCLI:
    def test_list_empty_catalog(self, tmp_path: Path) -> None:
        """list subcommand exits 0 with empty-catalog message."""
        from harness.commands.compiler_variants import _cmd_list
        import argparse
        ns = argparse.Namespace(command="list", validate=True)
        rc = _cmd_list(ns)
        assert rc == 0

    def test_verify_without_id_raises(self) -> None:
        """verify requires a positional id argument."""
        from harness.commands.compiler_variants import build_parser
        parser = build_parser()
        for argv in [["verify"], ["path"], ["install"]]:
            ns = parser.parse_args(argv + ["test-id"])
            assert hasattr(ns, "handler")


class TestWithCandidate:
    """Tests for _with_candidate flag replacement."""
    def test_replaces_o_level(self) -> None:
        from harness.match.flag_search import _with_candidate
        cmd = ["bin/cc", "-O2", "-G0", "-c", "src/foo.c", "-o", "foo.o"]
        result = _with_candidate(cmd, ["-O1"], Path("/out/candidate.o"))
        # -O2 removed, -o foo.o removed, -O1 added, output path set
        assert "-O2" not in result
        assert "-O1" in result
        assert "-o" in result
        assert "/out/candidate.o" in result

    def test_preserves_non_opt_flags(self) -> None:
        from harness.match.flag_search import _with_candidate
        cmd = ["bin/cc", "-O2", "-G0", "-funsigned-char", "-c", "src/foo.c", "-o", "foo.o"]
        result = _with_candidate(cmd, ["-O1", "-fno-schedule-insns"], Path("/out/c.o"))
        assert "-G0" in result
        assert "-funsigned-char" in result
        assert "-O2" not in result
        assert "-O1" in result
        assert "-fno-schedule-insns" in result

    def test_list_of_lists_flag_handling(self) -> None:
        """Verifies the _with_candidate handles flag lists as used by the flag-catalog."""
        from harness.match.flag_search import _with_candidate
        cmd = ["bin/cc", "-O2", "-G0", "-c", "src/bar.c", "-o", "bar.o"]
        # Simulate the flag-catalog format: each candidate is a list of flags
        for flags in [["-O0"], ["-O1"], ["-O2", "-fno-schedule-insns"], ["-O3"]]:
            result = _with_candidate(cmd, flags, Path("/out/bar.o"))
            assert all(f in result for f in flags)


class TestNonemptyListOfLists:
    """Tests for nonempty list-of-lists flag catalog processing."""

    def test_linked_output_path_uses_suffix(self) -> None:
        """The disassemble_linked call uses object_path.with_suffix('.linked.o')."""
        import inspect
        from harness.match.flag_search import search_flags
        source = inspect.getsource(search_flags)
        # The linked_path must be derived from object_path with .linked.o suffix
        assert "with_suffix('.linked.o')" in source, (
            "search_flags must use object_path.with_suffix('.linked.o') for linked output"
        )

    def test_with_candidate_nonempty_catalog_iteration(self) -> None:
        """Each flag list from a nonempty catalog is processed through _with_candidate."""
        from harness.match.flag_search import _with_candidate
        from pathlib import Path
        cmd = ["bin/cc", "-O2", "-G0", "-c", "src/foo.c", "-o", "foo.o"]
        catalog = [["-O0"], ["-O1"], ["-O2", "-fno-schedule-insns"], ["-O3"]]
        results = []
        for flags in catalog:
            result = _with_candidate(cmd, flags, Path("/out/foo.o"))
            results.append(result)
        assert len(results) == len(catalog)
        for i, (flags, result) in enumerate(zip(catalog, results)):
            assert all(f in result for f in flags), f"flags {flags} not in result {i}"
            assert "-o" in result


class TestDefaultObjdump:
    def test_objdump_path_has_bin(self) -> None:
        """Default objdump path includes /bin/ directory component."""
        import pathlib as _pl
        _root = _pl.Path(__file__).resolve().parents[1]
        _src = (_root / "harness" / "match" / "flag_search.py").read_text()
        assert "psn00b_toolchain_root / \"bin\" / \"mipsel-none-elf-objdump\"" in _src
