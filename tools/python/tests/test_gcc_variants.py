"""Tests for config/compiler/variants.json schema validation."""

from __future__ import annotations

import json
from pathlib import Path
from types import SimpleNamespace

import pytest

from harness.toolchain.gcc_variants import (
    CompilerVariantEntry,
    EmptyCatalog,
    _validate_entry,
    load_variants,
    sha256_file,
)


def _minimal_valid_entry(overrides: dict | None = None) -> dict:
    entry = {
        "id": "gcc-2.7.2-psx",
        "label": "GCC 2.7.2 PSX",
        "url": "https://github.com/example/gcc-2.7.2-psx.tar.gz",
        "checksum": "sha256:" + "a" * 64,
        "archive_name": "gcc.tar.gz",
        "license": "GPL-2.0+",
        "source": "https://github.com/decompals/old-gcc",
        "host": "linux-x86_64",
        "identity": "mips-sony-psx-gcc",
        "assembler": "ASPSX 2.56 compatible",
        "executable_relpath": "gcc",
    }
    if overrides:
        entry.update(overrides)
    return entry


def _make_layout(root: Path) -> SimpleNamespace:
    return SimpleNamespace(
        root=root,
        toolchains_dir=root / "toolchains",
        gcc_variants_root=root / "toolchains" / "gcc-variants",
        downloads_dir=root / "downloads",
    )


class TestEmptyCatalog:
    def test_empty_catalog_properties(self) -> None:
        ec = EmptyCatalog()
        assert ec.id == "none"
        assert ec.label == "No variant"
        assert ec.url == ""
        assert ec.checksum == ""
        assert ec.archive_name == ""

    def test_empty_catalog_verify(self, tmp_path: Path) -> None:
        ec = EmptyCatalog()
        layout = _make_layout(tmp_path)
        assert ec.verify(layout) == "empty catalog"  # type: ignore[arg-type]


class TestValidateEntry:
    def test_valid_entry(self) -> None:
        _validate_entry(_minimal_valid_entry())

    def test_http_source_raises(self) -> None:
        entry = _minimal_valid_entry({"source": "http://example.com/gcc"})
        with pytest.raises(ValueError, match="must start with https://"):
            _validate_entry(entry)

    def _omit_key(self, key: str) -> dict:
        """Return minimal entry dict with one key removed."""
        return {k: v for k, v in _minimal_valid_entry().items() if k != key}

    def test_missing_id_raises(self) -> None:
        with pytest.raises(ValueError, match="missing required fields"):
            _validate_entry(self._omit_key("id"))

    def test_missing_label_raises(self) -> None:
        with pytest.raises(ValueError, match="missing required fields"):
            _validate_entry(self._omit_key("label"))

    def test_missing_url_raises(self) -> None:
        with pytest.raises(ValueError, match="missing required fields"):
            _validate_entry(self._omit_key("url"))

    def test_missing_checksum_raises(self) -> None:
        with pytest.raises(ValueError, match="missing required fields"):
            _validate_entry(self._omit_key("checksum"))

    def test_missing_archive_name_raises(self) -> None:
        with pytest.raises(ValueError, match="missing required fields"):
            _validate_entry(self._omit_key("archive_name"))

    def test_missing_license_raises(self) -> None:
        with pytest.raises(ValueError, match="missing required fields"):
            _validate_entry(self._omit_key("license"))

    def test_empty_id_raises(self) -> None:
        entry = _minimal_valid_entry({"id": ""})
        with pytest.raises(ValueError, match="'id' must be a non-empty string"):
            _validate_entry(entry)

    def test_unsafe_id_raises(self) -> None:
        entry = _minimal_valid_entry({"id": "../../evil"})
        with pytest.raises(ValueError, match="unsafe characters"):
            _validate_entry(entry)

    def test_http_url_raises(self) -> None:
        entry = _minimal_valid_entry({"url": "http://example.com/gcc.tar.gz"})
        with pytest.raises(ValueError, match="must start with https://"):
            _validate_entry(entry)

    def test_invalid_checksum_format_raises(self) -> None:
        entry = _minimal_valid_entry({"checksum": "md5:abc123"})
        with pytest.raises(ValueError, match="'checksum' must start with"):
            _validate_entry(entry)

    def test_invalid_checksum_length_raises(self) -> None:
        entry = _minimal_valid_entry({"checksum": "sha256:short"})
        with pytest.raises(ValueError, match="exactly 64"):
            _validate_entry(entry)

    def test_upper_hex_checksum_raises(self) -> None:
        entry = _minimal_valid_entry({"checksum": "sha256:" + "A" * 64})
        with pytest.raises(ValueError, match="exactly 64 lowercase hex"):
            _validate_entry(entry)

    def test_archive_name_with_path_raises(self) -> None:
        entry = _minimal_valid_entry({"archive_name": "subdir/gcc.tar.gz"})
        with pytest.raises(ValueError, match="plain basename"):
            _validate_entry(entry)

    def test_extra_keys_rejected(self) -> None:
        entry = _minimal_valid_entry({"extra_field": "should fail"})
        with pytest.raises(ValueError, match="unexpected keys"):
            _validate_entry(entry)

    def test_missing_executable_relpath_raises(self) -> None:
        entry = {k: v for k, v in _minimal_valid_entry().items() if k != "executable_relpath"}
        with pytest.raises(ValueError, match="missing required fields"):
            _validate_entry(entry)

    def test_absolute_executable_relpath_raises(self) -> None:
        entry = _minimal_valid_entry({"executable_relpath": "/usr/bin/gcc"})
        with pytest.raises(ValueError, match="must be relative"):
            _validate_entry(entry)

    def test_traversal_executable_relpath_raises(self) -> None:
        entry = _minimal_valid_entry({"executable_relpath": "../bin/gcc"})
        with pytest.raises(ValueError, match="must not contain"):
            _validate_entry(entry)

    def test_root_extra_keys_rejected(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        f = tmp_path / "config" / "compiler" / "variants.json"
        f.parent.mkdir(parents=True)
        f.write_text(json.dumps({
            "schema": "harness.compiler-variants/v1",
            "candidates": [],
            "unknown_key": "bad",
        }))
        with pytest.raises(ValueError, match="unexpected keys"):
            load_variants(layout)

    def test_duplicate_id_raises(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        f = tmp_path / "config" / "compiler" / "variants.json"
        f.parent.mkdir(parents=True)
        f.write_text(json.dumps({
            "schema": "harness.compiler-variants/v1",
            "candidates": [
                _minimal_valid_entry({"id": "dup-id"}),
                _minimal_valid_entry({"id": "dup-id"}),
            ],
        }))
        with pytest.raises(ValueError, match="duplicate"):
            load_variants(layout)


class TestCompilerVariantEntry:
    def test_properties(self) -> None:
        entry = CompilerVariantEntry(_minimal_valid_entry({
            "id": "test-variant",
            "label": "Test Variant",
            "url": "https://example.com/test.tar.gz",
            "checksum": "sha256:" + "b" * 64,
            "archive_name": "test.tar.gz",
        }))
        assert entry.id == "test-variant"
        assert entry.label == "Test Variant"
        assert entry.url == "https://example.com/test.tar.gz"
        assert entry.checksum == "sha256:" + "b" * 64
        assert entry.archive_name == "test.tar.gz"
        assert entry.identity == "mips-sony-psx-gcc"
        assert entry.executable_relpath == "gcc"

    def test_install_path_uses_gcc_variants_root(self) -> None:
        entry = CompilerVariantEntry(_minimal_valid_entry({"id": "my-gcc"}))
        layout = _make_layout(Path("/tmp"))
        path = entry.install_path(layout)
        assert str(path) == str(Path("/tmp/toolchains/gcc-variants/my-gcc"))


class TestLoadVariants:
    def test_empty_catalog(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        result = load_variants(layout)
        assert result == []

    def test_no_file_returns_empty(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        result = load_variants(layout)
        assert result == []

    def test_invalid_schema_raises(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        variants_file = tmp_path / "config" / "compiler" / "variants.json"
        variants_file.parent.mkdir(parents=True)
        variants_file.write_text(json.dumps({
            "schema": "invalid/schema",
            "candidates": [],
        }))
        with pytest.raises(ValueError, match="expected schema"):
            load_variants(layout)

    def test_verify_checks_escaped_executable(self, tmp_path: Path) -> None:
        entry = CompilerVariantEntry(_minimal_valid_entry())
        layout = _make_layout(tmp_path)
        root = layout.gcc_variants_root / entry.id
        root.mkdir(parents=True)
        import os
        outside = tmp_path / "outside_gcc"
        outside.write_text("gcc\n")
        outside.chmod(0o755)
        os.symlink(str(outside), str(root / "gcc"))
        with pytest.raises((ValueError, FileNotFoundError)):
            entry.verify(layout)

    def test_candidates_not_list_raises(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        variants_file = tmp_path / "config" / "compiler" / "variants.json"
        variants_file.parent.mkdir(parents=True)
        variants_file.write_text(json.dumps({
            "schema": "harness.compiler-variants/v1",
            "candidates": "not a list",
        }))
        with pytest.raises(ValueError, match="candidates' must be a list"):
            load_variants(layout)

    def test_root_not_object_raises(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        variants_file = tmp_path / "config" / "compiler" / "variants.json"
        variants_file.parent.mkdir(parents=True)
        variants_file.write_text(json.dumps(["not an object"]))
        with pytest.raises(ValueError, match="must be a JSON object"):
            load_variants(layout)

    def test_note_not_list_raises(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        variants_file = tmp_path / "config" / "compiler" / "variants.json"
        variants_file.parent.mkdir(parents=True)
        variants_file.write_text(json.dumps({
            "schema": "harness.compiler-variants/v1",
            "note": "not a list",
            "candidates": [],
        }))
        with pytest.raises(ValueError, match="'note' must be a list"):
            load_variants(layout)

    def test_note_item_not_string_raises(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        variants_file = tmp_path / "config" / "compiler" / "variants.json"
        variants_file.parent.mkdir(parents=True)
        variants_file.write_text(json.dumps({
            "schema": "harness.compiler-variants/v1",
            "note": ["valid note", 42],
            "candidates": [],
        }))
        with pytest.raises(ValueError, match="must be a string"):
            load_variants(layout)

    def test_absent_note_passes(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        variants_file = tmp_path / "config" / "compiler" / "variants.json"
        variants_file.parent.mkdir(parents=True)
        variants_file.write_text(json.dumps({
            "schema": "harness.compiler-variants/v1",
            "candidates": [],
        }))
        result = load_variants(layout)
        assert result == []


class TestLoadVariantsStrictSchema:
    def test_missing_required_fields_raises(self, tmp_path: Path) -> None:
        layout = _make_layout(tmp_path)
        variants_file = tmp_path / "config" / "compiler" / "variants.json"
        variants_file.parent.mkdir(parents=True)
        variants_file.write_text(json.dumps({
            "schema": "harness.compiler-variants/v1",
            "candidates": [{"id": "x", "url": "https://x.com/x.tar.gz"}],
        }))
        with pytest.raises(ValueError, match="missing required fields"):
            load_variants(layout)


class TestSha256File:
    def test_sha256_computes_correctly(self, tmp_path: Path) -> None:
        test_file = tmp_path / "test.bin"
        test_file.write_bytes(b"hello world")
        result = sha256_file(test_file)
        assert result.startswith("sha256:")
        hex_part = result[len("sha256:"):]
        assert len(hex_part) == 64
        assert result == "sha256:b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9"


class TestSafeExtract:
    def test_safe_extract_raises_on_absolute(self, tmp_path: Path) -> None:
        import tarfile
        from harness.toolchain.gcc_variants import _safe_extract_tar_gz
        archive = tmp_path / "bad.tar.gz"
        with tarfile.open(archive, "w:gz") as tf:
            from io import BytesIO
            info = tarfile.TarInfo("/etc/passwd")
            info.size = 0
            tf.addfile(info, BytesIO())
        with pytest.raises(ValueError, match="absolute path"):
            _safe_extract_tar_gz(archive, tmp_path / "dest")

    def test_safe_extract_raises_on_traversal(self, tmp_path: Path) -> None:
        import tarfile
        from harness.toolchain.gcc_variants import _safe_extract_tar_gz
        archive = tmp_path / "bad.tar.gz"
        with tarfile.open(archive, "w:gz") as tf:
            from io import BytesIO
            info = tarfile.TarInfo("../escape")
            info.size = 0
            tf.addfile(info, BytesIO())
        with pytest.raises(ValueError, match="'..'"):
            _safe_extract_tar_gz(archive, tmp_path / "dest")

    def test_safe_extract_raises_on_fifo(self, tmp_path: Path) -> None:
        import tarfile
        from harness.toolchain.gcc_variants import _safe_extract_tar_gz
        archive = tmp_path / "fifo.tar.gz"
        with tarfile.open(archive, "w:gz") as tf:
            info = tarfile.TarInfo("named-pipe")
            info.type = tarfile.FIFOTYPE
            info.size = 0
            tf.addfile(info)
        with pytest.raises(ValueError, match="device"):
            _safe_extract_tar_gz(archive, tmp_path / "dest")

    def test_safe_extract_raises_on_symlink(self, tmp_path: Path) -> None:
        import tarfile
        from harness.toolchain.gcc_variants import _safe_extract_tar_gz
        archive = tmp_path / "bad.tar.gz"
        with tarfile.open(archive, "w:gz") as tf:
            info = tarfile.TarInfo("link")
            info.type = tarfile.SYMTYPE
            info.linkname = "/etc/passwd"
            info.size = 0
            tf.addfile(info)
        with pytest.raises(ValueError, match="link entry"):
            _safe_extract_tar_gz(archive, tmp_path / "dest")


class TestHostValidation:
    def test_valid_host_format(self) -> None:
        from harness.toolchain.gcc_variants import _validate_entry
        for host in ("linux-x86_64", "darwin-arm64", "win32-x86_64", "freebsd-amd64"):
            _validate_entry(_minimal_valid_entry({"host": host}))

    def test_empty_host_raises(self) -> None:
        from harness.toolchain.gcc_variants import _validate_entry
        with pytest.raises(ValueError, match="host.*non-empty"):
            _validate_entry(_minimal_valid_entry({"host": ""}))

    def test_invalid_host_format_raises(self) -> None:
        from harness.toolchain.gcc_variants import _validate_entry
        for host in ("linux", "linux-", "-x86_64", "OSX-ARM", "windows-64"):
            with pytest.raises(ValueError, match="invalid host format"):
                _validate_entry(_minimal_valid_entry({"host": host}))

    def test_check_host_compatible_valid_format(self) -> None:
        """check_host_compatible validates format of host string."""
        from harness.toolchain.gcc_variants import check_host_compatible
        check_host_compatible("linux-x86_64")

    def test_check_host_compatible_bad_format_raises(self) -> None:
        from harness.toolchain.gcc_variants import check_host_compatible
        with pytest.raises(ValueError, match="invalid host format"):
            check_host_compatible("windows-64")


class TestInstallCachedVerification:
    def test_cached_install_calls_verify(self, tmp_path: Path) -> None:
        """Cached install path calls verify() which checks host, identity, containment.

        Without a real extracted gcc binary, verify raises FileNotFoundError,
        proving the cached path does not short-circuit verification.
        """
        from harness.toolchain.gcc_variants import CompilerVariantEntry, sha256_file

        entry = CompilerVariantEntry(_minimal_valid_entry({"id": "test-gcc"}))
        layout = _make_layout(tmp_path)

        # Create a digest-valid archive
        archive = layout.downloads_dir / entry.archive_name
        archive.parent.mkdir(parents=True)
        archive.write_bytes(b"fake tarball content")
        entry._entry["checksum"] = sha256_file(archive)

        # No extracted gcc — verify raises FileNotFoundError
        with pytest.raises((FileNotFoundError, ValueError, RuntimeError)):
            entry.install(layout)
