"""Tests for config/compiler/variants.json schema and the shared GCC archive lifecycle."""

from __future__ import annotations

import io
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
        private_assets_dir=root / "inputs" / "external" / "private-assets",
        gcc_archive_cache_dir=root
        / "inputs"
        / "external"
        / "private-assets"
        / "toolchains"
        / "gcc",
    )


def _make_fake_gcc_archive(path: Path, *, version: str = "2.6.3") -> None:
    """Build a tar.gz containing an executable fake ``gcc`` printing identity."""
    import io
    import tarfile

    script = f"#!/bin/sh\necho 'mips-sony-psx-gcc (GCC) {version}'\n"
    with tarfile.open(path, "w:gz") as archive:
        info = tarfile.TarInfo("gcc")
        data = script.encode()
        info.size = len(data)
        info.mode = 0o755
        archive.addfile(info, io.BytesIO(data))


class _FakeResponse(io.BytesIO):
    """BytesIO stand-in for urllib responses under ``with``."""

    def __enter__(self) -> "_FakeResponse":
        return self

    def __exit__(self, *exc: object) -> None:
        self.close()


@pytest.fixture
def linux_x86_64(monkeypatch: pytest.MonkeyPatch) -> None:
    """Hermetic host: declare the running platform as linux-x86_64."""
    import sys

    from harness.toolchain import gcc_variants

    monkeypatch.setattr(sys, "platform", "linux")
    monkeypatch.setattr(gcc_variants._platform, "machine", lambda: "x86_64")


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

    def test_empty_catalog_install_raises(self, tmp_path: Path) -> None:
        ec = EmptyCatalog()
        layout = _make_layout(tmp_path)
        with pytest.raises(RuntimeError, match="no package to install"):
            ec.install(layout)  # type: ignore[arg-type]

    def test_empty_catalog_verify_identity_raises(self, tmp_path: Path) -> None:
        ec = EmptyCatalog()
        layout = _make_layout(tmp_path)
        with pytest.raises(RuntimeError, match="no binary to verify"):
            ec.verify_identity(layout)  # type: ignore[arg-type]


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
        for host in ("linux-x86_64", "darwin-arm64", "win32-x86_64"):
            _validate_entry(_minimal_valid_entry({"host": host}))

    def test_empty_host_raises(self) -> None:
        from harness.toolchain.gcc_variants import _validate_entry
        with pytest.raises(ValueError, match="host.*non-empty"):
            _validate_entry(_minimal_valid_entry({"host": ""}))

    def test_invalid_host_format_raises(self) -> None:
        from harness.toolchain.gcc_variants import _validate_entry
        for host in ("linux", "linux-", "-x86_64", "OSX-ARM", "windows-64", "freebsd-amd64"):
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

    def test_check_host_compatible_matching_host_and_arch(
        self, monkeypatch: pytest.MonkeyPatch
    ) -> None:
        """Hermetic: linux-x86_64 (and amd64 alias) passes under mocked linux+x86_64."""
        import sys
        from harness.toolchain import gcc_variants
        monkeypatch.setattr(sys, "platform", "linux")
        monkeypatch.setattr(gcc_variants._platform, "machine", lambda: "x86_64")
        gcc_variants.check_host_compatible("linux-x86_64")
        gcc_variants.check_host_compatible("linux-amd64")

    def test_check_host_compatible_arm_alias_and_mismatch(
        self, monkeypatch: pytest.MonkeyPatch
    ) -> None:
        """Hermetic: arm64/aarch64 aliases pass; x86_64 mismatches ARM."""
        import sys
        from harness.toolchain import gcc_variants
        monkeypatch.setattr(sys, "platform", "linux")
        monkeypatch.setattr(gcc_variants._platform, "machine", lambda: "aarch64")
        gcc_variants.check_host_compatible("linux-arm64")
        gcc_variants.check_host_compatible("linux-aarch64")
        with pytest.raises(RuntimeError, match="host mismatch"):
            gcc_variants.check_host_compatible("linux-x86_64")

    def test_check_host_compatible_rejects_os_mismatch(
        self, monkeypatch: pytest.MonkeyPatch
    ) -> None:
        """Hermetic: declared darwin-x86_64 fails under mocked linux+x86_64."""
        import sys
        from harness.toolchain import gcc_variants
        monkeypatch.setattr(sys, "platform", "linux")
        monkeypatch.setattr(gcc_variants._platform, "machine", lambda: "x86_64")
        with pytest.raises(RuntimeError, match="host mismatch"):
            gcc_variants.check_host_compatible("darwin-x86_64")


class TestInstallCachedVerification:
    def test_digest_valid_cache_recovers_missing_install(
        self, tmp_path: Path, linux_x86_64: None
    ) -> None:
        """Digest-valid cached archive + missing install -> install extracts it."""
        from harness.toolchain.gcc_variants import CompilerVariantEntry, sha256_file

        entry = CompilerVariantEntry(_minimal_valid_entry({"id": "test-gcc"}))
        layout = _make_layout(tmp_path)
        cache_dir = layout.gcc_archive_cache_dir
        cache_dir.mkdir(parents=True)
        cached = cache_dir / entry.archive_name
        _make_fake_gcc_archive(cached)
        entry._entry["checksum"] = sha256_file(cached)

        status = entry.install(layout)
        assert "installed and verified" in status
        assert (entry.install_path(layout) / "gcc").is_file()
        entry.verify(layout)

    def test_corrupt_cache_entry_replaced_by_verified_download(
        self, tmp_path: Path, monkeypatch: pytest.MonkeyPatch, linux_x86_64: None
    ) -> None:
        """A digest-mismatched cache entry is replaced by a fresh verified download."""
        from harness.toolchain import gcc_archive
        from harness.toolchain.gcc_variants import CompilerVariantEntry, sha256_file

        entry = CompilerVariantEntry(_minimal_valid_entry({"id": "test-gcc"}))
        layout = _make_layout(tmp_path)
        valid = tmp_path / "valid.tar.gz"
        _make_fake_gcc_archive(valid)
        entry._entry["checksum"] = sha256_file(valid)
        cache_dir = layout.gcc_archive_cache_dir
        cache_dir.mkdir(parents=True)
        cached = cache_dir / entry.archive_name
        cached.write_bytes(b"corrupt cache bytes")

        monkeypatch.setattr(
            gcc_archive.urllib.request,
            "urlopen",
            lambda url, timeout=120: _FakeResponse(valid.read_bytes()),
        )
        entry.install(layout)
        assert sha256_file(cached) == entry.checksum
        assert (entry.install_path(layout) / "gcc").is_file()

    def test_download_failure_leaves_no_cache_artifacts(
        self, tmp_path: Path, monkeypatch: pytest.MonkeyPatch, linux_x86_64: None
    ) -> None:
        """A failed download removes the temp file and leaves the cache clean."""
        from harness.toolchain import gcc_archive
        from harness.toolchain.gcc_variants import CompilerVariantEntry

        entry = CompilerVariantEntry(_minimal_valid_entry({"id": "test-gcc"}))
        layout = _make_layout(tmp_path)
        cache_dir = layout.gcc_archive_cache_dir
        cache_dir.mkdir(parents=True)

        def _raise(url: str, timeout: int = 120) -> None:
            raise OSError(f"network down for {url}")

        monkeypatch.setattr(gcc_archive.urllib.request, "urlopen", _raise)
        with pytest.raises(OSError, match="network down"):
            entry.install(layout)
        assert list(cache_dir.iterdir()) == []

    def test_failed_staged_identity_preserves_prior_install(
        self, tmp_path: Path, monkeypatch: pytest.MonkeyPatch, linux_x86_64: None
    ) -> None:
        """A failed identity check on the staged copy keeps the prior install."""
        from pathlib import Path as _Path

        from harness.toolchain import gcc_archive
        from harness.toolchain.gcc_variants import CompilerVariantEntry, sha256_file

        entry = CompilerVariantEntry(
            _minimal_valid_entry({"id": "test-gcc", "identity": "2.6.3"})
        )
        layout = _make_layout(tmp_path)
        valid = tmp_path / "valid.tar.gz"
        _make_fake_gcc_archive(valid, version="2.6.3")
        entry._entry["checksum"] = sha256_file(valid)
        cache_dir = layout.gcc_archive_cache_dir
        cache_dir.mkdir(parents=True)
        (cache_dir / entry.archive_name).write_bytes(valid.read_bytes())

        entry.install(layout)  # prior verified install
        exe = entry.install_path(layout) / "gcc"
        before = exe.read_bytes()

        def _staging_version_fails(exe_path: _Path, label: str) -> str:
            if ".staging-" in str(exe_path):
                raise ValueError(f"{label}: staged identity check failed")
            return "mips-sony-psx-gcc (GCC) 2.6.3"

        monkeypatch.setattr(gcc_archive, "_version_output", _staging_version_fails)
        with pytest.raises(ValueError, match="staged identity"):
            entry.install(layout, force=True)
        assert exe.read_bytes() == before
        assert not list((exe.parent.parent).glob(".*.backup-*"))
        assert not list(exe.parent.glob(".*-staging-*"))

    def test_cache_root_symlink_rejected(
        self, tmp_path: Path, linux_x86_64: None
    ) -> None:
        """A symlinked GCC cache root is rejected, never followed."""
        from harness.toolchain.gcc_variants import CompilerVariantEntry

        entry = CompilerVariantEntry(_minimal_valid_entry({"id": "test-gcc"}))
        layout = _make_layout(tmp_path)
        real = tmp_path / "real-cache"
        real.mkdir()
        layout.gcc_archive_cache_dir.parent.mkdir(parents=True)
        layout.gcc_archive_cache_dir.symlink_to(real, target_is_directory=True)
        with pytest.raises(ValueError, match="symlinked GCC cache root"):
            entry.install(layout)

    def test_cache_entry_symlink_rejected(
        self, tmp_path: Path, linux_x86_64: None
    ) -> None:
        """A symlinked cache entry is rejected, never downloaded over or followed."""
        from harness.toolchain.gcc_variants import CompilerVariantEntry

        entry = CompilerVariantEntry(_minimal_valid_entry({"id": "test-gcc"}))
        layout = _make_layout(tmp_path)
        cache_dir = layout.gcc_archive_cache_dir
        cache_dir.mkdir(parents=True)
        outside = tmp_path / "elsewhere"
        outside.write_bytes(b"not an archive")
        (cache_dir / entry.archive_name).symlink_to(outside)
        with pytest.raises(ValueError, match="symlinked cached archive"):
            entry.install(layout)

    def test_cache_entry_non_regular_rejected(
        self, tmp_path: Path, linux_x86_64: None
    ) -> None:
        """A non-regular cache entry (directory) is rejected."""
        from harness.toolchain.gcc_variants import CompilerVariantEntry

        entry = CompilerVariantEntry(_minimal_valid_entry({"id": "test-gcc"}))
        layout = _make_layout(tmp_path)
        cache_dir = layout.gcc_archive_cache_dir
        cache_dir.mkdir(parents=True)
        (cache_dir / entry.archive_name).mkdir()
        with pytest.raises(ValueError, match="not a regular file"):
            entry.install(layout)

    def test_verify_installed_rejects_symlinked_install_root(
        self, tmp_path: Path
    ) -> None:
        """verify_installed rejects a symlinked install root before is_file."""
        from harness.toolchain import gcc_archive

        real = tmp_path / "real-install"
        real.mkdir()
        (real / "gcc").write_text("#!/bin/sh\necho ok\n")
        dest = tmp_path / "install"
        dest.symlink_to(real, target_is_directory=True)
        with pytest.raises(ValueError, match="symlinked install root"):
            gcc_archive.verify_installed(
                dest=dest,
                executable_relpath="gcc",
                expected_identity="ok",
                label="test",
            )

    def test_verify_installed_rejects_symlinked_executable(
        self, tmp_path: Path
    ) -> None:
        """verify_installed rejects a symlinked executable before resolve."""
        from harness.toolchain import gcc_archive

        dest = tmp_path / "install"
        dest.mkdir()
        outside = tmp_path / "outside-gcc"
        outside.write_text("#!/bin/sh\necho ok\n")
        (dest / "gcc").symlink_to(outside)
        with pytest.raises(ValueError, match="symlinked executable"):
            gcc_archive.verify_installed(
                dest=dest,
                executable_relpath="gcc",
                expected_identity="ok",
                label="test",
            )


class TestEnsureVariant:
    def test_missing_install_auto_installs(
        self, tmp_path: Path, linux_x86_64: None
    ) -> None:
        """ensure_variant installs a missing selected install from the cache."""
        from harness.toolchain.gcc_variants import (
            CompilerVariantEntry,
            ensure_variant,
            sha256_file,
        )

        entry = CompilerVariantEntry(_minimal_valid_entry({"id": "test-gcc"}))
        layout = _make_layout(tmp_path)
        valid = tmp_path / "valid.tar.gz"
        _make_fake_gcc_archive(valid)
        entry._entry["checksum"] = sha256_file(valid)
        cache_dir = layout.gcc_archive_cache_dir
        cache_dir.mkdir(parents=True)
        (cache_dir / entry.archive_name).write_bytes(valid.read_bytes())

        resolved = ensure_variant(layout, entry)
        expected = (entry.install_path(layout) / "gcc").resolve()
        assert Path(resolved) == expected

    def test_corrupt_install_fails_closed(
        self, tmp_path: Path, linux_x86_64: None
    ) -> None:
        """An existing-but-unverifiable install raises; no host/canonical fallback."""
        from harness.toolchain.gcc_variants import (
            CompilerVariantEntry,
            ensure_variant,
        )

        entry = CompilerVariantEntry(_minimal_valid_entry({"id": "test-gcc"}))
        layout = _make_layout(tmp_path)
        dest = entry.install_path(layout)
        dest.mkdir(parents=True)
        (dest / "junk").write_text("not a compiler")
        with pytest.raises(RuntimeError, match="corrupt or incomplete"):
            ensure_variant(layout, entry)

    def test_unsupported_host_fails_closed(
        self, tmp_path: Path, monkeypatch: pytest.MonkeyPatch
    ) -> None:
        """A host-incompatible selected variant fails closed in ensure_variant."""
        import sys

        from harness.toolchain import gcc_variants

        monkeypatch.setattr(sys, "platform", "linux")
        monkeypatch.setattr(gcc_variants._platform, "machine", lambda: "aarch64")
        entry = gcc_variants.CompilerVariantEntry(
            _minimal_valid_entry({"host": "linux-x86_64"})
        )
        layout = _make_layout(tmp_path)
        with pytest.raises(RuntimeError, match="host mismatch"):
            gcc_variants.ensure_variant(layout, entry)


class TestPathAndCompileCommandsParity:
    def test_cmd_path_auto_installs_missing_variant(
        self, tmp_path: Path, monkeypatch: pytest.MonkeyPatch, linux_x86_64: None
    ) -> None:
        """`compiler-variants path <id>` installs a missing selected install."""
        from harness.commands import compiler_variants as cmd
        from harness.commands.compiler_variants import _cmd_path

        layout = _make_layout(tmp_path)
        monkeypatch.setattr(cmd, "repo_layout", lambda: layout)
        entry = _minimal_valid_entry({"id": "test-gcc"})
        valid = tmp_path / "valid.tar.gz"
        _make_fake_gcc_archive(valid)
        entry["checksum"] = sha256_file(valid)
        catalog = tmp_path / "config" / "compiler" / "variants.json"
        catalog.parent.mkdir(parents=True)
        catalog.write_text(
            json.dumps(
                {"schema": "harness.compiler-variants/v1", "candidates": [entry]}
            )
        )
        cache_dir = layout.gcc_archive_cache_dir
        cache_dir.mkdir(parents=True)
        (cache_dir / entry["archive_name"]).write_bytes(valid.read_bytes())
        args = SimpleNamespace(id="test-gcc")
        assert _cmd_path(args) == 0
        assert (layout.gcc_variants_root / "test-gcc" / "gcc").is_file()

    def test_compile_commands_auto_installs_selected_variant(
        self, tmp_path: Path, monkeypatch: pytest.MonkeyPatch, linux_x86_64: None
    ) -> None:
        """compile_commands.json emits PSX_GCC and installs a missing selection."""
        from harness.commands import compile_commands as cc_cmd

        layout = _make_layout(tmp_path)
        monkeypatch.setattr(cc_cmd, "repo_layout", lambda root=None: layout)
        (tmp_path / "src" / "game").mkdir(parents=True)
        (tmp_path / "src" / "game" / "a.c").write_text("int x;\n")
        objflags = tmp_path / "config" / "compiler" / "object-flags.cmake"
        objflags.parent.mkdir(parents=True)
        objflags.write_text("set(BOF3_OBJCOMPILER_game_a_c test-gcc)\n")
        entry = _minimal_valid_entry({"id": "test-gcc"})
        valid = tmp_path / "valid.tar.gz"
        _make_fake_gcc_archive(valid)
        entry["checksum"] = sha256_file(valid)
        catalog = tmp_path / "config" / "compiler" / "variants.json"
        catalog.write_text(
            json.dumps(
                {"schema": "harness.compiler-variants/v1", "candidates": [entry]}
            )
        )
        cache_dir = layout.gcc_archive_cache_dir
        cache_dir.mkdir(parents=True)
        (cache_dir / entry["archive_name"]).write_bytes(valid.read_bytes())

        assert cc_cmd.run(SimpleNamespace(root=tmp_path)) == 0
        payload = json.loads((tmp_path / "compile_commands.json").read_text())
        arguments = payload[0]["arguments"]
        assert arguments[:3] == ["cmake", "-E", "env"]
        psx_gcc = [arg for arg in arguments if arg.startswith("PSX_GCC=")]
        assert len(psx_gcc) == 1
        expected = (layout.gcc_variants_root / "test-gcc" / "gcc").resolve()
        assert Path(psx_gcc[0].split("=", 1)[1]) == expected
