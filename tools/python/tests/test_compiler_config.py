"""Tests for compiler configuration helpers."""

from __future__ import annotations

from pathlib import Path
from types import SimpleNamespace

import pytest

from harness.compiler_config import (
    load_object_flags,
    load_object_compilers,
    sanitize_identifier,
)


def _make_layout(root: Path) -> SimpleNamespace:
    return SimpleNamespace(
        root=root,
        toolchains_dir=root / "toolchains",
        gcc_variants_root=root / "toolchains" / "gcc-variants",
        downloads_dir=root / "downloads",
    )


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
