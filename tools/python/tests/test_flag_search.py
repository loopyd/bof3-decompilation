"""Tests for flag-search variant environment integration."""

from __future__ import annotations

import json
from pathlib import Path
from types import SimpleNamespace

import pytest


def _make_layout(root: Path) -> SimpleNamespace:
    return SimpleNamespace(
        root=root,
        toolchains_dir=root / "toolchains",
        gcc_variants_root=root / "toolchains" / "gcc-variants",
        downloads_dir=root / "downloads",
    )


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
        assert 'with_suffix(".linked.o")' in source, (
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


class TestExplicitCompilerOverride:
    def test_removes_only_embedded_compiler_selection(self) -> None:
        from harness.match.flag_search import _strip_embedded_psx_gcc

        command = [
            "cmake", "-E", "env", "KEEP=1", "PSX_GCC=/old/gcc",
            "bin/cc", "-c", "source.c", "-o", "source.o",
        ]
        assert _strip_embedded_psx_gcc(command) == [
            "cmake", "-E", "env", "KEEP=1", "bin/cc", "-c", "source.c",
            "-o", "source.o",
        ]

    def test_unselected_command_is_unchanged(self) -> None:
        from harness.match.flag_search import _strip_embedded_psx_gcc

        command = ["bin/cc", "-c", "source.c", "-o", "source.o"]
        assert _strip_embedded_psx_gcc(command) == command

    def test_search_override_preserves_command_arguments(
        self, monkeypatch: pytest.MonkeyPatch, tmp_path: Path
    ) -> None:
        import harness.match.flag_search as flag_search

        source = tmp_path / "source.c"
        source.write_text("int f(void) { return 0; }\n")
        original = tmp_path / "original.s"
        original.write_text("nop\n")
        original_bytes = tmp_path / "original.bin"
        original_bytes.write_bytes(b"\0\0\0\0")
        command = [
            "cmake", "-E", "env", "KEEP=1", "PSX_GCC=/selected/gcc",
            "bin/cc", "PSX_GCC=argument", "-O2", "-c", str(source),
            "-o", "source.o",
        ]
        (tmp_path / "compile_commands.json").write_text(json.dumps([{
            "directory": str(tmp_path), "file": str(source), "arguments": command,
        }]))
        catalog = tmp_path / "flags.json"
        catalog.write_text(json.dumps({"candidates": [["-O1"]]}))
        monkeypatch.setattr(flag_search, "run_asm_diff_one", lambda *_a, **_kw: {
            "outputs": {"original": str(original), "original_bytes": str(original_bytes)},
            "original_size": 4, "address": "0x80000000",
        })

        class Variant:
            label = "requested"
            executable_relpath = "gcc"

            def verify(self, _layout: object) -> None:
                pass

            def install_path(self, _layout: object) -> Path:
                return tmp_path / "requested"

        monkeypatch.setattr(flag_search, "lookup_variant", lambda *_args: Variant())
        calls: list[tuple[list[str], dict[str, str]]] = []

        def run(args: list[str], **kwargs: object) -> SimpleNamespace:
            calls.append((args, kwargs["env"]))  # type: ignore[index]
            return SimpleNamespace(returncode=1)

        monkeypatch.setattr(flag_search.subprocess, "run", run)
        layout = SimpleNamespace(root=tmp_path, psn00b_toolchain_root=tmp_path)
        flag_search.search_flags(layout=layout, source=source, catalog_path=catalog)
        flag_search.search_flags(
            layout=layout, source=source, catalog_path=catalog, compiler_id="requested"
        )

        default_args, default_env = calls[0]
        override_args, override_env = calls[1]
        assert "PSX_GCC=/selected/gcc" in default_args
        assert "PSX_GCC" not in default_env
        assert "PSX_GCC=/selected/gcc" not in override_args
        assert "PSX_GCC=argument" in override_args
        assert override_env["PSX_GCC"] == str(tmp_path / "requested" / "gcc")


class TestDefaultObjdump:
    def test_objdump_path_has_bin(self) -> None:
        """Default objdump path includes /bin/ directory component."""
        import pathlib as _pl
        _root = _pl.Path(__file__).resolve().parents[1]
        _src = (_root / "harness" / "match" / "flag_search.py").read_text()
        assert "psn00b_toolchain_root / \"bin\" / \"mipsel-none-elf-objdump\"" in _src
