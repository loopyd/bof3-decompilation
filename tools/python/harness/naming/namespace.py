"""Select and resolve naming evidence namespaces and artifact paths."""

from __future__ import annotations

import hashlib
import os
from contextvars import ContextVar
from pathlib import Path

_EVIDENCE_ROOT: ContextVar[Path | None] = ContextVar(
    "naming_evidence_root", default=None
)


def canonical_evidence_root(value: str | Path) -> Path:
    """Return one canonically spelled absolute root with no symlink ancestors."""

    spelling = os.fspath(value)
    parts = spelling.split("/")
    if (
        not spelling.startswith("/")
        or spelling.startswith("//")
        or spelling == "/"
        or spelling.endswith("/")
        or any(part in {"", ".", ".."} for part in parts[1:])
        or spelling != os.path.normpath(spelling)
    ):
        raise ValueError(
            "--evidence-root requires one canonical absolute path "
            "(no //, trailing slash, '.', '..', or filesystem root '/')"
        )
    path = Path(spelling)
    current = Path(path.anchor)
    for part in path.parts[1:]:
        current /= part
        try:
            if current.lstat().st_mode & 0o170000 == 0o120000:
                raise ValueError("explicit evidence root must not traverse a symlink")
        except FileNotFoundError:
            break
    return path


def set_evidence_root(path: Path | None = None):
    """Temporarily select a canonical absolute, symlink-free evidence parent."""

    if path is not None:
        path = canonical_evidence_root(path)
    return _EVIDENCE_ROOT.set(path)


def reset_evidence_root(token) -> None:
    """Restore the previous runner artifact root."""

    _EVIDENCE_ROOT.reset(token)


def selected_evidence_root() -> Path | None:
    """Return the active explicit evidence parent, if any."""

    return _EVIDENCE_ROOT.get()


def journal_dir(root: Path, report: Path, target: str) -> Path:
    """Evidence namespace under the selected parent, keyed by report digest."""

    digest = hashlib.sha256(report.read_bytes()).hexdigest()[:12]
    selected = _EVIDENCE_ROOT.get()
    parent = selected or (root / "out" / "reviews" / "evidence").resolve()
    safe_target = target.replace("/", "__") if selected else target
    namespace = (parent / f"{safe_target}__{digest}").resolve()
    if parent != namespace and parent not in namespace.parents:
        raise ValueError("evidence namespace escapes selected root")
    return namespace


def artifact_root(root: Path, report: Path, target: str) -> Path:
    """Return the runner-owned artifact namespace for one report identity."""

    return journal_dir(root, report, target)


def journal_path(root: Path, report: Path, target: str) -> Path:
    return journal_dir(root, report, target) / "checkpoint.jsonl"


def manifest_path(root: Path, report: Path, target: str) -> Path:
    return journal_dir(root, report, target) / "manifest.json"


def _artifact_path(root: Path, namespace: Path, value: object) -> Path:
    if not isinstance(value, str) or not value:
        raise ValueError("evidence artifact path must be non-empty")
    candidate = Path(value)
    explicit = selected_evidence_root() is not None
    if candidate.is_absolute() != explicit:
        raise ValueError("evidence artifact path does not match namespace mode")
    path = (candidate if explicit else root / candidate).resolve()
    if namespace not in path.parents:
        raise ValueError("evidence artifact escapes selected namespace")
    return path


def _artifact_is_fresh(
    root: Path, namespace: Path, relative: object, digest: object, size: object
) -> bool:
    if (
        not isinstance(digest, str)
        or not isinstance(size, int)
        or isinstance(size, bool)
        or size < 0
    ):
        return False
    try:
        path = _artifact_path(root, namespace, relative)
    except ValueError:
        return False
    if not path.is_file():
        return False
    data = path.read_bytes()
    return len(data) == size and hashlib.sha256(data).hexdigest() == digest
