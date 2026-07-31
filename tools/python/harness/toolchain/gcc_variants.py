"""Variant catalog schema, validator, and resolver for per-object compiler profiles."""

from __future__ import annotations

import hashlib
import json
import platform as _platform
import re
import subprocess
import sys
import tarfile
import urllib.request
from abc import ABC, abstractmethod
from pathlib import Path
from typing import Any

from ..io import RepoLayout


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
        exe = self.install_path(layout) / self.executable_relpath
        if not exe.is_file():
            raise FileNotFoundError(f"missing {self.id}: {exe} not found")
        resolved = exe.resolve()
        root = self.install_path(layout).resolve()
        if not (resolved == root or root in resolved.parents):
            raise ValueError(
                f"{self.id}: executable path {resolved} escapes variant root {root}"
            )
        version = self.verify_identity(layout)
        if self.identity and self.identity not in version:
            raise ValueError(
                f"{self.id}: --version output does not contain expected identity "
                f"{self.identity!r}"
            )
        return self.label

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
    r"^(linux|darwin|freebsd|win32)-(x86_64|i686|aarch64|arm64|amd64)$"
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
    """Verify the declared host matches the running platform."""
    if not _HOST_RE.match(host):
        raise ValueError(
            f"invalid host format {host!r}; expected <os>-<arch>"
        )
    sys_os = {"linux": "linux", "darwin": "darwin", "win32": "win32"}.get(sys.platform)
    if sys_os is None:
        raise RuntimeError(f"unsupported platform: {sys.platform}")
    declared_os = host.split("-")[0]
    if sys_os != declared_os:
        raise RuntimeError(
            f"host mismatch: declared {host!r}, running {sys_os}-{_platform.machine()}"
        )


def _safe_extract_tar_gz(archive_path: Path, dest: Path) -> None:
    """Extract tar.gz rejecting absolute, traversal, device/FIFO, and link entries."""
    dest.mkdir(parents=True, exist_ok=True)
    with tarfile.open(archive_path, "r:gz") as tf:
        for member in tf.getmembers():
            if member.issym() or member.islnk():
                raise ValueError(
                    f"archive contains link entry {member.name!r}; rejecting for safety"
                )
            if member.isdev() or member.isfifo():
                raise ValueError(
                    f"archive contains device entry {member.name!r}; rejecting"
                )
            name = Path(member.name)
            if name.is_absolute():
                raise ValueError(
                    f"archive contains absolute path {member.name!r}; rejecting"
                )
            if ".." in name.parts:
                raise ValueError(
                    f"archive contains path with '..' {member.name!r}; rejecting"
                )
            tf.extract(member, dest, filter="data")


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
        """Download, verify digest, and extract this variant."""
        check_host_compatible(self.host)
        archive = layout.downloads_dir / self.archive_name
        if archive.is_file() and not force:
            existing = sha256_file(archive)
            if existing == self.checksum:
                dest = self.install_path(layout)
                self.verify(layout)
                return f"{self.id}: already installed"

        # Download
        archive.parent.mkdir(parents=True, exist_ok=True)
        urllib.request.urlretrieve(self.url, archive)

        # Verify digest
        actual = sha256_file(archive)
        if actual != self.checksum:
            archive.unlink()
            raise ValueError(
                f"{self.id}: SHA-256 mismatch "
                f"(expected {self.checksum}, got {actual})"
            )

        # Safe extract
        dest = self.install_path(layout)
        if dest.exists():
            import shutil
            shutil.rmtree(dest)
        _safe_extract_tar_gz(archive, dest)

        # Verify executable is a regular file under the variant root
        exe = dest / self.executable_relpath
        if not exe.is_file():
            raise FileNotFoundError(
                f"{self.id}: {self.executable_relpath} not found after extraction"
            )
        resolved = exe.resolve()
        if not (resolved == dest.resolve() or dest.resolve() in resolved.parents):
            raise ValueError(
                f"{self.id}: executable path {resolved} escapes variant root {dest}"
            )

        # Run full verification: file exists, root containment, identity
        self.verify(layout)
        return f"{self.id}: installed and verified"


def sha256_file(path: Path) -> str:
    """Compute SHA-256 hex digest of a file."""
    h = hashlib.sha256()
    h.update(path.read_bytes())
    return f"sha256:{h.hexdigest()}"
