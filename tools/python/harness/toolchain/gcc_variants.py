"""Variant catalog schema, validator, and resolver for per-object compiler profiles."""

from __future__ import annotations

import json
import platform as _platform
import re
import subprocess
import sys
from abc import ABC, abstractmethod
from pathlib import Path
from typing import Any
from ..io import RepoLayout
from .gcc_archive import (
    _safe_extract_tar_gz,  # noqa: F401 — re-exported for tests
    install_archive,
    sha256_file,  # noqa: F401 — re-exported for tests
    verify_installed,
)


class CompilerVariant(ABC):
    """A single historical GCC compiler variant entry."""

    @property
    @abstractmethod
    def id(self) -> str:
        """Unique identifier for this variant."""

    @property
    @abstractmethod
    def label(self) -> str:
        """Human-readable name for display."""

    @property
    @abstractmethod
    def url(self) -> str:
        """Download URL for the archive containing this variant's binaries."""

    @property
    @abstractmethod
    def checksum(self) -> str:
        """SHA-256 of the downloaded archive."""

    @property
    @abstractmethod
    def archive_name(self) -> str:
        """Archive filename (e.g., 'gcc-2.7.2-psx.tar.gz')."""

    @abstractmethod
    def install(self, layout: RepoLayout, *, force: bool = False) -> str:
        """Download, verify digest, and install this variant."""

    @abstractmethod
    def install_path(self, layout: RepoLayout) -> Path:
        """Where this variant installs relative to the toolchains directory."""

    @property
    @abstractmethod
    def identity(self) -> str:
        """Expected identity substring in --version output (e.g. 'mips-sony-psx-gcc')."""

    @property
    @abstractmethod
    def executable_relpath(self) -> str:
        """Relative path from variant root to the compiler binary (e.g. 'gcc')."""

    @property
    @abstractmethod
    def host(self) -> str:
        """Expected host platform string."""

    def verify(self, layout: RepoLayout) -> str:
        """Verify the installed variant: file exists, within root, identity matches."""
        check_host_compatible(self.host)
        return verify_installed(
            dest=self.install_path(layout),
            executable_relpath=self.executable_relpath,
            expected_identity=self.identity,
            label=self.label,
        )

    def verify_identity(self, layout: RepoLayout) -> str:
        """Verify the installed binary's --version output contains expected text."""
        exe = self.install_path(layout) / self.executable_relpath
        if not exe.is_file():
            raise FileNotFoundError(f"missing {self.id}: {exe} not found")
        result = subprocess.run(
            [str(exe), "--version"],
            capture_output=True, text=True, timeout=30,
        )
        if result.returncode != 0:
            raise RuntimeError(f"{self.id}: gcc --version exited {result.returncode}")
        return result.stdout.strip()


class EmptyCatalog(CompilerVariant):
    """Sentinel indicating no valid candidates exist in the catalog."""

    @property
    def id(self) -> str:
        return "none"

    @property
    def label(self) -> str:
        return "No variant"

    @property
    def url(self) -> str:
        return ""

    @property
    def checksum(self) -> str:
        return ""

    @property
    def archive_name(self) -> str:
        return ""

    @property
    def host(self) -> str:
        return ""

    @property
    def identity(self) -> str:
        return ""

    @property
    def executable_relpath(self) -> str:
        return ""

    def install(self, layout: RepoLayout, *, force: bool = False) -> str:
        raise RuntimeError("empty catalog has no package to install")

    def install_path(self, layout: RepoLayout) -> Path:
        return Path("/dev/null")

    def verify(self, layout: RepoLayout) -> str:
        return "empty catalog"

    def verify_identity(self, layout: RepoLayout) -> str:
        raise RuntimeError("empty catalog has no binary to verify")


_SCHEMA = "harness.compiler-variants/v1"
_ALLOWED_ROOT_KEYS = {"schema", "note", "candidates"}
_REQUIRED_FIELDS = {
    "id", "label", "url", "checksum", "archive_name",
    "license", "source", "host", "identity", "assembler",
    "executable_relpath",
}
_SAFE_ID_RE = re.compile(r"^[a-zA-Z0-9][a-zA-Z0-9._-]*$")
_SAFE_RELPATH_RE = re.compile(r"^[a-zA-Z0-9_][-a-zA-Z0-9_./]*$")
_HOST_RE = re.compile(
    r"^(linux|darwin|win32)-(x86_64|i686|aarch64|arm64|amd64)$"
)


def _reject_extra_keys(obj: dict[str, Any], allowed: set[str], label: str) -> None:
    extras = set(obj.keys()) - allowed
    if extras:
        raise ValueError(f"{label}: unexpected keys: {sorted(extras)}")


def _validate_entry(entry: dict[str, Any]) -> None:
    missing = _REQUIRED_FIELDS - set(entry.keys())
    if missing:
        raise ValueError(f"variant entry missing required fields: {sorted(missing)}")
    _reject_extra_keys(entry, _REQUIRED_FIELDS, "variant entry")
    if not isinstance(entry["id"], str) or not entry["id"]:
        raise ValueError("variant 'id' must be a non-empty string")
    if not _SAFE_ID_RE.match(entry["id"]):
        raise ValueError(
            f"variant 'id' {entry['id']!r} contains unsafe characters; "
            f"use only [a-zA-Z0-9._-]"
        )
    if not isinstance(entry["label"], str) or not entry["label"]:
        raise ValueError("variant 'label' must be a non-empty string")
    if not isinstance(entry["url"], str) or not entry["url"]:
        raise ValueError("variant 'url' must be a non-empty string")
    if not entry["url"].startswith("https://"):
        raise ValueError("variant 'url' must start with https://")
    if not isinstance(entry["checksum"], str) or not entry["checksum"]:
        raise ValueError("variant 'checksum' must be a non-empty string")
    if not entry["checksum"].startswith("sha256:"):
        raise ValueError("variant 'checksum' must start with 'sha256:'")
    hex_part = entry["checksum"][len("sha256:"):]
    if not re.match(r"^[0-9a-f]{64}$", hex_part):
        raise ValueError(
            "variant 'checksum' must contain exactly 64 lowercase hex digits"
        )
    if not isinstance(entry["archive_name"], str) or not entry["archive_name"]:
        raise ValueError("variant 'archive_name' must be a non-empty string")
    if "/" in entry["archive_name"] or entry["archive_name"] in (".", ".."):
        raise ValueError("variant 'archive_name' must be a plain basename")
    if not isinstance(entry.get("license", ""), str) or not entry["license"]:
        raise ValueError("variant 'license' must be a non-empty string")
    if not isinstance(entry.get("source", ""), str) or not entry["source"]:
        raise ValueError("variant 'source' must be a non-empty string")
    if not entry["source"].startswith("https://"):
        raise ValueError("variant 'source' must start with https://")
    if not isinstance(entry.get("host", ""), str) or not entry["host"]:
        raise ValueError("variant 'host' must be a non-empty string")
    if not _HOST_RE.match(entry["host"]):
        raise ValueError(
            f"invalid host format {entry['host']!r}; expected <os>-<arch>"
        )
    if not isinstance(entry.get("identity", ""), str) or not entry["identity"]:
        raise ValueError("variant 'identity' must be a non-empty string")
    if not isinstance(entry.get("assembler", ""), str) or not entry["assembler"]:
        raise ValueError("variant 'assembler' must be a non-empty string")
    if not isinstance(entry.get("executable_relpath", ""), str) or not entry["executable_relpath"]:
        raise ValueError("variant 'executable_relpath' must be a non-empty string")
    if entry["executable_relpath"].startswith("/"):
        raise ValueError("variant 'executable_relpath' must be relative (no leading /)")
    if ".." in Path(entry["executable_relpath"]).parts:
        raise ValueError("variant 'executable_relpath' must not contain '..'")
    if not _SAFE_RELPATH_RE.match(entry["executable_relpath"]):
        raise ValueError(
            f"variant 'executable_relpath' {entry['executable_relpath']!r} contains "
            f"unsafe characters; use only [-a-zA-Z0-9_./]"
        )


def check_host_compatible(host: str) -> None:
    """Verify the declared host matches the running platform (OS and arch).

    Normalizes obvious aliases (amd64 == x86_64, arm64 == aarch64) before
    comparing; rejects any OS or architecture mismatch.
    """
    if not _HOST_RE.match(host):
        raise ValueError(
            f"invalid host format {host!r}; expected <os>-<arch>"
        )
    sys_os = {"linux": "linux", "darwin": "darwin", "win32": "win32"}.get(sys.platform)
    if sys_os is None:
        raise RuntimeError(f"unsupported platform: {sys.platform}")
    arch_alias = {"amd64": "x86_64", "arm64": "aarch64"}
    machine = _platform.machine().lower()
    sys_arch = arch_alias.get(machine, machine)
    declared_os, declared_arch = host.split("-", 1)
    declared_arch = arch_alias.get(declared_arch, declared_arch)
    if sys_os != declared_os or sys_arch != declared_arch:
        raise RuntimeError(
            f"host mismatch: declared {host!r}, running {sys_os}-{sys_arch}"
        )


def ensure_variant(layout: RepoLayout, variant: CompilerVariant) -> str:
    """Resolve the verified GCC executable path, auto-installing when absent.

    Fails closed: an unsupported host, unknown ID, corrupt existing install,
    or failed install raises instead of falling back to the canonical or host
    GCC. ``compiler-variants path <id>`` and ``compile_commands.py`` both use
    this so a selected variant is installed on demand when only its install
    is missing.
    """
    check_host_compatible(variant.host)
    dest = variant.install_path(layout)
    exe = dest / variant.executable_relpath
    if exe.is_file():
        variant.verify(layout)
        return str(exe.resolve())
    if dest.exists():
        raise RuntimeError(
            f"{variant.id}: existing installation at {dest} is corrupt or "
            f"incomplete; run `bin/compiler-variants install --force {variant.id}`"
        )
    variant.install(layout)
    variant.verify(layout)
    return str(exe.resolve())


def lookup_variant(layout: RepoLayout, compiler_id: str) -> CompilerVariant:
    """Look up a specific compiler ID from the catalog.

    Returns the matching variant or raises ValueError if not found.
    """
    variants = load_variants(layout, validate=True)
    for v in variants:
        if v.id == compiler_id:
            return v
    raise ValueError(
        f"compiler variant {compiler_id!r} not found in catalog"
    )


def load_variants(
    layout: RepoLayout, *, validate: bool = True
) -> list[CompilerVariant]:
    """Load and validate the variant catalog.

    Returns an empty list when the catalog is valid but has no candidates.
    Raises ValueError on schema violation.
    """
    path = layout.root / "config" / "compiler" / "variants.json"
    if not path.is_file():
        return []
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict):
        raise ValueError("catalog root must be a JSON object")
    _reject_extra_keys(payload, _ALLOWED_ROOT_KEYS, "catalog root")
    if payload.get("schema") != _SCHEMA:
        raise ValueError(f"expected schema {_SCHEMA}, got {payload.get('schema')}")
    note = payload.get("note")
    if note is not None:
        if not isinstance(note, list):
            raise ValueError("catalog 'note' must be a list")
        for i, item in enumerate(note):
            if not isinstance(item, str):
                raise ValueError(f"catalog 'note[{i}]' must be a string")
    entries = payload.get("candidates", [])
    if not isinstance(entries, list):
        raise ValueError("catalog 'candidates' must be a list")
    if validate:
        seen_ids: set[str] = set()
        for entry in entries:
            _validate_entry(entry)
            cid = entry["id"]
            if cid in seen_ids:
                raise ValueError(f"duplicate variant ID: {cid!r}")
            seen_ids.add(cid)
    return [CompilerVariantEntry(e) for e in entries]


class CompilerVariantEntry(CompilerVariant):
    """Concrete implementation backed by a catalog entry dict."""

    def __init__(self, entry: dict[str, Any]) -> None:
        self._entry = entry

    @property
    def id(self) -> str:
        return self._entry["id"]

    @property
    def label(self) -> str:
        return self._entry["label"]

    @property
    def url(self) -> str:
        return self._entry["url"]

    @property
    def checksum(self) -> str:
        return self._entry["checksum"]

    @property
    def archive_name(self) -> str:
        return self._entry["archive_name"]

    @property
    def host(self) -> str:
        return self._entry["host"]

    @property
    def identity(self) -> str:
        return self._entry["identity"]

    @property
    def executable_relpath(self) -> str:
        return self._entry["executable_relpath"]

    def install_path(self, layout: RepoLayout) -> Path:
        return layout.gcc_variants_root / self.id

    def install(self, layout: RepoLayout, *, force: bool = False) -> str:
        """Download, verify digest, and install this variant."""
        check_host_compatible(self.host)
        return install_archive(
            layout,
            archive_name=self.archive_name,
            url=self.url,
            checksum=self.checksum,
            dest=self.install_path(layout),
            executable_relpath=self.executable_relpath,
            expected_identity=self.identity,
            label=f"variant {self.id}",
            force=force,
        )
