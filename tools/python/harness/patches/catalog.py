"""Discover bounded text patches from inputs/patches target folders."""

from __future__ import annotations

import hashlib
from pathlib import Path
import re

from .git import run_git
from .models import Patch, Target

NAME = re.compile(r"[a-z0-9][a-z0-9_.-]*")


def regular_path(path: Path) -> None:
    if (
        path.resolve() != path.absolute()
        or not path.is_file()
        or path.stat().st_nlink != 1
    ):
        raise ValueError(f"patch input must be a regular, nonlinked file: {path}")


def read_patch_inputs(
    folder: Path, *, allow_empty: bool = False
) -> list[tuple[Path, bytes]]:
    if folder.resolve() != folder.absolute() or not folder.is_dir():
        raise ValueError(f"missing or symlinked patch folder: {folder}")
    paths = sorted(folder.glob("*.patch"))
    if (not paths and not allow_empty) or len(paths) > 64:
        raise ValueError(f"target requires 1..64 patch files: {folder.name}")
    inputs, total = [], 0
    for path in paths:
        if not NAME.fullmatch(path.stem):
            raise ValueError(f"invalid patch name: {path.name}")
        regular_path(path)
        with path.open("rb") as source:
            data = source.read(2 * 1024 * 1024 + 1)
        total += len(data)
        if len(data) > 2 * 1024 * 1024 or total > 16 * 1024 * 1024 or b"\0" in data:
            raise ValueError("patch input exceeds text/size bounds")
        inputs.append((path, data))
    return inputs


def discover_patches(target: Target) -> list[Patch]:
    if len(set(target.patch_order)) != len(target.patch_order):
        raise ValueError(f"duplicate patch order entry: {target.name}")
    patches = []
    for path, data in read_patch_inputs(target.root / "inputs/patches" / target.name):
        summary = run_git(target.source, "apply", "--summary", data=data).decode()
        if any(word in summary for word in ("mode change", "rename ", "copy ")) or any(
            " mode " in line
            and not (" mode 100644 " in line or "delete mode 100755 " in line)
            for line in summary.splitlines()
        ):
            raise ValueError(
                f"patch mode/rename/copy changes are unsupported: {path.name}"
            )
        files = []
        for record in run_git(
            target.source, "apply", "--numstat", "-z", data=data
        ).split(b"\0"):
            if not record:
                continue
            added, removed, raw_name = record.split(b"\t", 2)
            name = raw_name.decode()
            relative = Path(name)
            if (
                added == b"-"
                or removed == b"-"
                or not name
                or relative.is_absolute()
                or any(part in {"..", ".git"} for part in relative.parts)
                or str(relative) != name
                or "\n" in name
                or "\r" in name
            ):
                raise ValueError(f"unsupported binary or unsafe patch path: {name!r}")
            files.append(name)
        if not files or len(files) != len(set(files)):
            raise ValueError(
                f"patch has no files or duplicate file records: {path.name}"
            )
        patches.append(
            Patch(path.name, path, hashlib.sha256(data).hexdigest(), tuple(files), data)
        )
    order = {name: index for index, name in enumerate(target.patch_order)}
    return sorted(
        patches, key=lambda patch: (order.get(patch.name, len(order)), patch.name)
    )
