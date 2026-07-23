"""Stage authorized BOF3 media and local reverse-engineering dependencies."""

from __future__ import annotations

import argparse
import shutil
import subprocess
import tomllib
from pathlib import Path

from ..domain import load_target_manifests
from ..emi.catalog import load_catalog, materialize_reviewed_targets
from ..emi.operations import emi_unpack
from ..io import repo_layout
from ..toolchain.psx import install_canonical_psx_toolchain
from ..toolchain.disc import find_disc_set, import_bof3_disc
from ..toolchain.psyq import import_psyq_sdk, stage_psyq_converted_sdk
from ._common import run_main
from .compile_commands import run as write_compile_commands


REQUIRED_TOOLS = (
    "bin/as",
    "bin/ld",
    "bin/ar",
    "bin/nm",
    "bin/objcopy",
    "bin/objdump",
    "bin/ranlib",
    "bin/strip",
)


def _run(command: list[str], *, cwd: Path, quiet: bool = False) -> None:
    result = subprocess.run(
        command,
        cwd=cwd,
        check=False,
        stdout=subprocess.PIPE if quiet else None,
        stderr=subprocess.STDOUT if quiet else None,
        text=quiet,
    )
    if result.returncode:
        output = result.stdout.strip() if quiet and result.stdout else ""
        raise RuntimeError(
            f"command failed ({result.returncode}): {' '.join(command)}"
            + (f"\n{output}" if output else "")
        )


def _find_or_import_disc(root: Path, *, force: bool) -> Path:
    disc_root = root / "inputs" / "external"
    try:
        cue, _ = find_disc_set(disc_root)
        return cue
    except FileNotFoundError:
        return import_bof3_disc(dest=disc_root, force=force).cue_path


def _build_local_tools(root: Path) -> None:
    layout = repo_layout(root)
    for source, target in (
        (layout.harness_disk_src, layout.harness_disk_bin.parent.parent),
        (layout.emi_ex_src, layout.emi_ex_bin.parent.parent),
    ):
        _run(
            [
                "cargo",
                "build",
                "--locked",
                "--release",
                "--manifest-path",
                str(source / "Cargo.toml"),
                "--target-dir",
                str(target),
            ],
            cwd=root,
        )


def _materialize_executables(root: Path, *, force: bool) -> None:
    extracted = (root / "out" / "extracted").resolve()
    for manifest in load_target_manifests(root).values():
        if manifest.kind != "executable":
            continue
        source = (extracted / manifest.disc_id).resolve()
        if not source.is_relative_to(extracted) or not source.is_file():
            raise FileNotFoundError(f"missing extracted executable for {manifest.id}: {source}")
        destination = root / manifest.binary
        if destination.exists() and not force:
            continue
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)


def _extract_and_materialize(root: Path, cue: Path, *, force: bool) -> None:
    _run(
        [str(root / "bin" / "bof3-disk"), "extract", "-i", str(cue), "-o", "out/extracted"],
        cwd=root,
    )
    emi_unpack(
        tool_path=root / "bin" / "emi-ex",
        cwd=root,
        extracted_dir=root / "out" / "extracted",
        raw_emi_dir=root / "out" / "extracted",
    )
    _materialize_executables(root, force=force)
    materialize_reviewed_targets(root=root, catalog=load_catalog(root))


def _psyq_47_members(root: Path) -> list[Path]:
    members: set[Path] = set()
    for manifest in (root / "config" / "targets").rglob("target.toml"):
        data = tomllib.loads(manifest.read_text(encoding="utf-8"))
        libraries = data.get("psyq", {}).get("libraries", {})
        for library in libraries.values():
            for member in library.get("members", []):
                path = Path(member)
                if path.parts[:2] == ("psyq", "4.7"):
                    members.add(root / "toolchains" / path)
    return sorted(members)


def verify_setup(root: Path) -> None:
    layout = repo_layout(root)
    required = (
        layout.gcc272_psx_root / "gcc",
        layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-as",
        layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-ld",
        layout.psyq_root / "include" / "libgpu.h",
        layout.psyq_root / "lib",
        layout.harness_disk_bin,
        layout.emi_ex_bin,
        root / "third_party" / "maspsx" / "maspsx.py",
    )
    missing = [str(path.relative_to(root)) for path in required if not path.exists()]
    if missing:
        raise FileNotFoundError("missing staged files: " + ", ".join(missing))
    _run([str(root / "bin" / "cc"), "-x", "c", "-E", "-"], cwd=root, quiet=True)
    for tool in REQUIRED_TOOLS:
        _run([str(root / tool), "--version"], cwd=root, quiet=True)
    cue, cue_tracks = find_disc_set(root / "inputs" / "external")
    if not cue_tracks or not all(track.is_file() for track in cue_tracks):
        raise FileNotFoundError(f"incomplete BIN/CUE set: {cue}")
    missing_images = [
        str(manifest.binary)
        for manifest in load_target_manifests(root).values()
        if not (root / manifest.binary).is_file()
    ]
    if missing_images:
        raise FileNotFoundError("missing target images: " + ", ".join(missing_images))
    missing_members = [
        str(path.relative_to(root)) for path in _psyq_47_members(root) if not path.is_file()
    ]
    if missing_members:
        raise FileNotFoundError("missing PsyQ 4.7 members: " + ", ".join(missing_members))
    _run([str(root / "bin" / "bof3-disk"), "--example"], cwd=root, quiet=True)
    _run([str(root / "bin" / "emi-ex"), "--example"], cwd=root, quiet=True)


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    _run(["git", "submodule", "update", "--init", "--recursive"], cwd=root)
    layout = repo_layout(root)
    cue = _find_or_import_disc(root, force=args.force)
    install_canonical_psx_toolchain(layout, force=args.force)
    import_psyq_sdk(
        dest=layout.psyq_root,
        archive=args.psyq_archive,
        archive_url=args.psyq_url,
        private_assets_root=layout.private_assets_dir,
        force=args.force,
    )
    stage_psyq_converted_sdk(
        dest=layout.psyq_root,
        private_assets_root=layout.private_assets_dir,
        force=args.force,
    )
    _build_local_tools(root)
    write_compile_commands(argparse.Namespace(root=root))
    _extract_and_materialize(root, cue, force=args.force)
    verify_setup(root)
    print("setup: complete")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="setup",
        description="Stage authorized BOF3 media and required toolchains.",
    )
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    parser.add_argument("--psyq-archive", type=Path, help="PsyQ 4.7 archive under inputs/")
    parser.add_argument("--psyq-url", help="explicit PsyQ 4.7 archive URL")
    parser.add_argument("--force", action="store_true")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
