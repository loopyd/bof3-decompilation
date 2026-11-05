"""Build a static page using an isolated checkout's fresh reverse index."""

from __future__ import annotations

import shutil
from pathlib import Path

from harness.analysis.index import rebuild
from harness.analysis.index_snapshot import snapshot_for
from harness.analysis.project import analyze_project
from harness.analysis.snapshot import snapshot_path
from harness.domain.manifests import load_target_manifests
from harness.pages.export import write_snapshot

ASSETS = ("index.html", "style.css", "app.js", "bof3-logo.webp")


def prepare_index(root: Path, cache: Path) -> None:
    for target, manifest in sorted(load_target_manifests(root).items()):
        binary = root / manifest.binary
        original = cache / manifest.binary
        if not original.is_file():
            raise ValueError(
                f"missing local binary: {manifest.binary}; run just binaries"
            )
        binary.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(original, binary)
        snapshot = snapshot_path(root, target)
        original_snapshot = snapshot_path(cache, target)
        if original_snapshot.is_file():
            snapshot.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(original_snapshot, snapshot)
        try:
            snapshot_for(root, target, binary, manifest=manifest)
        except (ValueError, FileNotFoundError):
            print(f"pages: refreshing snapshot {target}", flush=True)
            analyze_project(root, target)
    print("pages: rebuilding reverse index for pushed commit", flush=True)
    rebuild(root)


def build_site(
    root: Path, output: Path, revision: str, cache: Path | None = None
) -> None:
    if cache is not None:
        prepare_index(root, cache)
    output.mkdir(parents=True, exist_ok=True)
    write_snapshot(root, output / "status.json", revision)
    for name in ASSETS:
        shutil.copyfile(root / "web" / name, output / name)
    (output / ".nojekyll").touch()
