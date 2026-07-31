"""Tests for compiler configuration helpers."""

from __future__ import annotations

import json
import os
from pathlib import Path
from types import SimpleNamespace

import pytest

from harness.compiler_config import (
    resolve_compiler_variant,
    set_environment_for_variant,
    get_objcompiler_env,
    load_object_flags,
    load_object_compilers,
    sanitize_identifier,
)
from harness.toolchain.gcc_variants import EmptyCatalog


def _make_layout(root: Path) -> SimpleNamespace:
    return SimpleNamespace(
        root=root,
        toolchains_dir=root / "toolchains",
        gcc_variants_root=root / "toolchains" / "gcc-variants",
        downloads_dir=root / "downloads",
    )


class TestResolveCompilerVariant:
    def test_no_compiler_id_returns_empty_sentinel(self) -> None:
        from harness.io import RepoLayout
        import tempfile
        with tempfile.TemporaryDirectory() as tmp:
            layout = RepoLayout(
                root=Path(tmp),
                build_dir=Path(tmp) / "build",
                out_dir=Path(tmp) / "out",
                toolchains_dir=Path(tmp) / "toolchains",
                third_party_dir=Path(tmp) / "third_party",
                inputs_dir=Path(tmp) / "inputs",
                downloads_dir=Path(tmp) / "downloads",
                private_assets_dir=Path(tmp) / "inputs" / "external" / "private-assets",
                harness_disk_src=Path(tmp) / "tools" / "rust" / "bof3-disk",
                emi_ex_src=Path(tmp) / "tools" / "rust" / "emi-ex",
                harness_disk_bin=Path(tmp) / "tools" / "rust" / "bof3-disk" / "release" / "bof3-disk",
                emi_ex_bin=Path(tmp) / "tools" / "rust" / "emi-ex" / "release" / "emi-ex",
                psn00b_toolchain_root=Path(tmp) / "toolchains" / "psn00b_toolchain",
                psn00b_sdk_root=Path(tmp) / "toolchains" / "psn00bsdk",
                gcc272_psx_root=Path(tmp) / "toolchains" / "gcc-2.7.2-psx",
                gcc_variants_root=Path(tmp) / "toolchains" / "gcc-variants",
                psyq_root=Path(tmp) / "toolchains" / "psyq" / "4.7",
            )
            variant = resolve_compiler_variant(layout, compiler_id=None)
            assert isinstance(variant, EmptyCatalog)

    def test_nonexistent_id_raises(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        with pytest.raises(ValueError, match="not found in catalog"):
            resolve_compiler_variant(layout, compiler_id="nonexistent-id")  # type: ignore[arg-type]

    def test_empty_catalog_file_returns_empty(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        variants_file = tmp_path / "config" / "compiler" / "variants.json"
        variants_file.parent.mkdir(parents=True)
        variants_file.write_text(json.dumps({
            "schema": "harness.compiler-variants/v1",
            "candidates": [],
        }))
        variant = resolve_compiler_variant(layout, compiler_id=None)  # type: ignore[arg-type]
        assert isinstance(variant, EmptyCatalog)


class TestSetEnvironmentForVariant:
    def test_empty_catalog_returns_empty_dict(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        variant = EmptyCatalog()
        env = set_environment_for_variant(layout, variant)  # type: ignore[arg-type]
        assert env == {}

    def test_none_variant_returns_empty(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        env = set_environment_for_variant(layout, compiler_id=None)  # type: ignore[arg-type]
        assert env == {}


class TestGetObjcompilerEnv:
    def test_empty_env_returns_empty_dict(self) -> None:
        original = {}
        for key in list(os.environ.keys()):
            if key.startswith("BOF3_OBJCOMPILER"):
                original[key] = os.environ.pop(key)
        try:
            env = get_objcompiler_env()
            assert env == {}
        finally:
            os.environ.update(original)

    def test_set_var_is_retained(self) -> None:
        original = os.environ.get("BOF3_OBJCOMPILER")
        try:
            os.environ["BOF3_OBJCOMPILER"] = "/test/path"
            env = get_objcompiler_env()
            assert env.get("bof3_objcompiler") == "/test/path"
        finally:
            if original is not None:
                os.environ["BOF3_OBJCOMPILER"] = original
            else:
                os.environ.pop("BOF3_OBJCOMPILER", None)


class TestSanitizeIdentifier:
    def test_basic_relative_path(self) -> None:
        assert sanitize_identifier("battle/15.c") == "battle_15_c"

    def test_alphanumeric_unchanged(self) -> None:
        assert sanitize_identifier("abc123") == "abc123"

    def test_dots_become_underscore(self) -> None:
        assert sanitize_identifier("func_801D0D5C.c") == "func_801D0D5C_c"

    def test_dashes_become_underscore(self) -> None:
        assert sanitize_identifier("foo-bar") == "foo_bar"

    def test_mixed_path(self) -> None:
        result = sanitize_identifier("emi/battle/battle/03/func_801E29B4.c")
        assert result == "emi_battle_battle_03_func_801E29B4_c"


class TestLoadObjectFlags:
    def test_missing_file_returns_empty(self, tmp_path: Path) -> None:
        assert load_object_flags(tmp_path) == {}

    def test_parses_flags(self, tmp_path: Path) -> None:
        cmake = tmp_path / "config" / "compiler" / "object-flags.cmake"
        cmake.parent.mkdir(parents=True)
        cmake.write_text(
            "set(BOF3_OBJFLAGS_emi_etc_game_01_func_801D0D5C_c -O1)\n"
            "set(BOF3_OBJFLAGS_emi_battle_battle_15_func_800AB760_c -O2 -Wa,--expand-div)\n"
        )
        result = load_object_flags(tmp_path)
        assert result["emi_etc_game_01_func_801D0D5C_c"] == ["-O1"]
        assert result["emi_battle_battle_15_func_800AB760_c"] == ["-O2", "-Wa,--expand-div"]


class TestLoadObjectCompilers:
    def test_missing_file_returns_empty(self, tmp_path: Path) -> None:
        assert load_object_compilers(tmp_path) == {}

    def test_parses_compiler_ids(self, tmp_path: Path) -> None:
        cmake = tmp_path / "config" / "compiler" / "object-flags.cmake"
        cmake.parent.mkdir(parents=True)
        cmake.write_text(
            "set(BOF3_OBJCOMPILER_emi_battle_battle_15_func_800AB760_c gcc-2.6.3-psx)\n"
        )
        result = load_object_compilers(tmp_path)
        assert result["emi_battle_battle_15_func_800AB760_c"] == "gcc-2.6.3-psx"

    def test_empty_file_returns_empty(self, tmp_path: Path) -> None:
        cmake = tmp_path / "config" / "compiler" / "object-flags.cmake"
        cmake.parent.mkdir(parents=True)
        cmake.write_text("# only comments\n")
        assert load_object_compilers(tmp_path) == {}

    def test_duplicate_key_raises(self, tmp_path: Path) -> None:
        cmake = tmp_path / "config" / "compiler" / "object-flags.cmake"
        cmake.parent.mkdir(parents=True)
        cmake.write_text(
            "set(BOF3_OBJCOMPILER_my_key gcc-a)\n"
            "set(BOF3_OBJCOMPILER_my_key gcc-b)\n"
        )
        with pytest.raises(ValueError, match="duplicate"):
            load_object_compilers(tmp_path)

    def test_empty_value_raises(self, tmp_path: Path) -> None:
        cmake = tmp_path / "config" / "compiler" / "object-flags.cmake"
        cmake.parent.mkdir(parents=True)
        cmake.write_text(
            "set(BOF3_OBJCOMPILER_my_key )\n"
        )
        with pytest.raises(ValueError, match="malformed"):
            load_object_compilers(tmp_path)

    def test_multi_token_value_raises(self, tmp_path: Path) -> None:
        cmake = tmp_path / "config" / "compiler" / "object-flags.cmake"
        cmake.parent.mkdir(parents=True)
        cmake.write_text(
            "set(BOF3_OBJCOMPILER_my_key bad id!)\n"
        )
        with pytest.raises(ValueError, match="malformed"):
            load_object_compilers(tmp_path)

    def test_commented_line_ignored(self, tmp_path: Path) -> None:
        cmake = tmp_path / "config" / "compiler" / "object-flags.cmake"
        cmake.parent.mkdir(parents=True)
        cmake.write_text(
            "# set(BOF3_OBJCOMPILER_my_key gcc-a)\n"
        )
        assert load_object_compilers(tmp_path) == {}

    def test_no_compiler_entries_returns_empty(self, tmp_path: Path) -> None:
        cmake = tmp_path / "config" / "compiler" / "object-flags.cmake"
        cmake.parent.mkdir(parents=True)
        cmake.write_text(
            "set(BOF3_OBJFLAGS_my_key -O2)\n"
            "# BOF3_OBJCOMPILER only in a comment\n"
        )
        assert load_object_compilers(tmp_path) == {}
