"""Resolve scoped Markdown references without fetching URLs or following symlinks."""

from __future__ import annotations

from collections import Counter
import os
from pathlib import Path, PurePosixPath
import stat
from urllib.parse import unquote, urlsplit

from harness.common.directory import open_parent_fd
from harness.common.files import read_file
from harness.docs.anchors import collect_anchors
from harness.docs.documents import read_documents
from harness.docs.markdown import collect_links, decode_text
from harness.docs.paths import collect_document_paths, is_authored_path


BROKEN_STATUSES = {
    "undefined-reference",
    "invalid-target",
    "outside-root",
    "missing-target",
    "missing-fragment",
    "special-file",
}
UNCHECKED_STATUSES = {"symlink", "unreadable", "unchecked-fragment"}


def resolve_path(source: str, destination: str) -> str | None:
    parts = (
        [] if destination.startswith("/") else list(PurePosixPath(source).parent.parts)
    )
    for part in destination.split("/"):
        if part in {"", "."}:
            continue
        if part == "..":
            if not parts:
                return None
            parts.pop()
        else:
            parts.append(part)
    return "/".join(parts) or "."


def inspect_target(root: Path, name: str) -> str:
    if name == ".":
        return "directory"
    try:
        parent, leaf = open_parent_fd(root, name)
        try:
            details = os.stat(leaf, dir_fd=parent, follow_symlinks=False)
        finally:
            os.close(parent)
    except (FileNotFoundError, NotADirectoryError):
        return "missing-target"
    except ValueError:
        return "symlink"
    except OSError:
        return "unreadable"
    if stat.S_ISLNK(details.st_mode):
        return "symlink"
    if stat.S_ISDIR(details.st_mode):
        return "directory"
    return "file" if stat.S_ISREG(details.st_mode) else "special-file"


def resolve_link(
    root: Path, source: str, link: dict, anchors: dict[str, set[str] | None]
) -> dict:
    if link["destination"] is None:
        return {"status": "undefined-reference"}
    destination = decode_text(link["destination"])
    if any(ord(character) < 32 for character in destination) or "\\" in destination:
        return {"status": "invalid-target"}
    try:
        url = urlsplit(destination)
        if url.scheme or url.netloc:
            return {"status": "external", "url": destination}
        path = unquote(url.path, errors="strict")
        fragment = unquote(url.fragment, errors="strict")
    except (ValueError, UnicodeError):
        return {"status": "invalid-target"}
    if "\\" in path or any(ord(character) < 32 for character in path):
        return {"status": "invalid-target"}
    name = resolve_path(source, path) if path else source
    if name is None:
        return {"status": "outside-root"}
    result = {"resolved_path": name, "fragment": fragment, "query": url.query}
    kind = inspect_target(root, name)
    if kind not in {"directory", "file"}:
        return {**result, "status": kind}
    if path.endswith("/") and kind != "directory":
        return {**result, "status": "missing-target"}
    if not fragment:
        return {**result, "status": "ok", "target_kind": kind}
    if (
        kind != "file"
        or Path(name).suffix.lower() != ".md"
        or not is_authored_path(name)
    ):
        return {**result, "status": "unchecked-fragment", "target_kind": kind}
    if name not in anchors:
        try:
            anchors[name] = collect_anchors(read_file(root, name).decode("utf-8"))
        except (OSError, ValueError, UnicodeError):
            anchors[name] = None
    available = anchors[name]
    status = (
        "unchecked-fragment"
        if available is None
        else "ok"
        if fragment in available
        else "missing-fragment"
    )
    return {**result, "status": status, "target_kind": kind}


def inspect_references(root: Path, paths: list[str], *, broken_only: bool) -> dict:
    names, skipped = collect_document_paths(root, paths)
    documents = read_documents(root, names) if names else []
    anchors = {
        document["path"]: collect_anchors(document["text"]) for document in documents
    }
    references = []
    for document in documents:
        for link in collect_links(document["text"]):
            resolution = resolve_link(root, document["path"], link, anchors)
            references.append(
                {
                    "path": document["path"],
                    **link,
                    **resolution,
                    "broken": resolution["status"] in BROKEN_STATUSES,
                }
            )
    statuses = Counter(item["status"] for item in references)
    return {
        "documents": [
            {key: value for key, value in document.items() if key != "text"}
            for document in documents
        ],
        "references": [
            item for item in references if not broken_only or item["broken"]
        ],
        "reference_count": len(references),
        "broken_count": sum(statuses[status] for status in BROKEN_STATUSES),
        "unchecked_count": sum(statuses[status] for status in UNCHECKED_STATUSES),
        "status_counts": dict(sorted(statuses.items())),
        "broken_only": broken_only,
        "skipped": skipped,
    }
