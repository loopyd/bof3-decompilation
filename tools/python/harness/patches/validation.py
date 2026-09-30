"""Reconstruct patch series from pinned Git blobs and identify exact live states."""

from __future__ import annotations

from pathlib import Path
import stat
import tempfile

from .catalog import discover_patches
from .git import git_text, run_git
from .models import Patch, Series, Target

MAX_BYTES = 128 * 1024 * 1024


def read_sources(
    source: Path, names: set[str]
) -> tuple[dict[str, bytes | None], dict[str, int]]:
    contents, modes, total = {}, {}, 0
    for name in sorted(names):
        path = source / name
        if path.resolve() != path.absolute():
            raise ValueError(f"symlink in patch source path: {name}")
        if not path.exists():
            contents[name], modes[name] = None, 0o644
            continue
        info = path.stat()
        if (
            not stat.S_ISREG(info.st_mode)
            or info.st_nlink != 1
            or info.st_size > 16 * 1024 * 1024
        ):
            raise ValueError(f"patch source must be a bounded regular file: {name}")
        contents[name], modes[name] = path.read_bytes(), stat.S_IMODE(info.st_mode)
        total += len(contents[name])
        if total > MAX_BYTES:
            raise ValueError("patch source snapshot exceeds 128 MiB")
    return contents, modes


def collect_groups(patches: list[Patch]) -> list[list[int]]:
    groups = []
    for index, patch in enumerate(patches):
        touching = [
            group
            for group in groups
            if any(set(patch.files) & set(patches[i].files) for i in group)
        ]
        merged = [index]
        for group in touching:
            merged.extend(group)
            groups.remove(group)
        groups.append(sorted(merged))
    return sorted(groups, key=lambda group: group[0])


def inspect_series(target: Target) -> Series:
    if (
        target.source.resolve() != target.source.absolute()
        or not target.source.is_dir()
    ):
        raise ValueError(f"invalid patch target directory: {target.source}")
    if git_text(target.source, "rev-parse", "HEAD") != target.revision:
        raise ValueError(f"patch target revision differs from its pin: {target.name}")
    patches = discover_patches(target)
    names = {name for patch in patches for name in patch.files}
    if len(names) > 256:
        raise ValueError("patch series exceeds 256 files")
    changed = set(
        git_text(
            target.source,
            "diff",
            "--name-only",
            "--no-renames",
            "--ignore-submodules=none",
            "HEAD",
            "--",
        ).splitlines()
    )
    if changed - names:
        raise ValueError(
            f"target has tracked changes outside its patch series: {sorted(changed - names)}"
        )
    summary = git_text(target.source, "diff", "--summary", "HEAD", "--")
    if "mode change" in summary or "rename " in summary:
        raise ValueError("target has unexpected tracked mode or rename changes")
    evidence = target.inspect(names)
    observed, modes = read_sources(target.source, names)
    versions = []
    with tempfile.TemporaryDirectory(prefix="harness-patches-") as temporary:
        sandbox = Path(temporary)
        base = {}
        total = 0
        for name in sorted(names):
            entry = run_git(target.source, "ls-tree", "-z", target.revision, "--", name)
            if not entry:
                base[name] = None
                continue
            mode, kind, _ = entry.split(b"\t", 1)[0].split()
            if mode not in {b"100644", b"100755"} or kind != b"blob":
                raise ValueError(
                    f"patches cannot cross submodules or modify symlinks: {name}"
                )
            data = run_git(target.source, "show", f"{target.revision}:{name}")
            if len(data) > 16 * 1024 * 1024:
                raise ValueError(f"base file exceeds 16 MiB: {name}")
            base[name] = data
            total += len(data)
            path = sandbox / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
            if observed[name] is None:
                modes[name] = int(mode, 8) & 0o777
        versions.append(base)
        for patch in patches:
            run_git(sandbox, "apply", "--no-index", "--check", data=patch.data)
            run_git(sandbox, "apply", "--no-index", data=patch.data)
            after, _ = read_sources(sandbox, names)
            if after == versions[-1]:
                raise ValueError(f"patch has no observable change: {patch.name}")
            total += sum(len(data) for data in after.values() if data is not None)
            if total > MAX_BYTES:
                raise ValueError("patch series versions exceed 128 MiB")
            versions.append(after)
    groups, counts = collect_groups(patches), []
    for group in groups:
        files = {name for i in group for name in patches[i].files}
        candidates = [0] + [i + 1 for i in group]
        matches = [
            count
            for count, version in enumerate(candidates)
            if all(observed[name] == versions[version][name] for name in files)
        ]
        if len(matches) != 1:
            raise ValueError(
                f"target files are modified, partially patched or ambiguous in group: {[patches[i].name for i in group]}"
            )
        counts.append(matches[0])
    return Series(target, patches, versions, groups, counts, observed, modes, evidence)


def describe_series(series: Series) -> dict:
    applied = {
        i for group, count in zip(series.groups, series.counts) for i in group[:count]
    }
    return {
        "revision": series.target.revision,
        **series.evidence,
        "patches": [
            {
                "name": patch.name,
                "path": str(patch.path.relative_to(series.target.root)),
                "sha256": patch.sha256,
                "applied": i in applied,
            }
            for i, patch in enumerate(series.patches)
        ],
    }
