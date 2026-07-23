"""Stage authorized BOF3 media and local reverse-engineering dependencies."""

from __future__ import annotations

import argparse
import contextlib
import io
import shutil
import subprocess
import tomllib
from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from ..domain import load_target_manifests
from ..emi.catalog import load_catalog, materialize_reviewed_targets
from ..emi.operations import emi_unpack
from ..io import RepoLayout, repo_layout
from ..toolchain.disc import DiscToolchain, find_disc_set
from ..toolchain.gcc import GccToolchain
from ..toolchain.maspsx import MaspsxToolchain
from ..toolchain.psn00b import Psn00bToolchain
from ..toolchain.psyq import PsyqToolchain
from ..toolchain.asm_differ import AsmDifferToolchain
from ..toolchain.m2c import M2cToolchain
from ..toolchain.permuter import DecompPermuterToolchain
from ..toolchain.rizin import RizinToolchain
from ..toolchain.signatures import PsyqSignaturesToolchain
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


@dataclass
class SetupState:
    root: Path
    layout: RepoLayout
    args: argparse.Namespace
    cue: Path | None = None


SetupTaskRunner = Callable[[SetupState], str]


@dataclass(frozen=True)
class SetupTask:
    label: str
    run: SetupTaskRunner


TASKS: list[SetupTask] = []


def setup_task(label: str) -> Callable[[SetupTaskRunner], SetupTaskRunner]:
    def register(run: SetupTaskRunner) -> SetupTaskRunner:
        TASKS.append(SetupTask(label, run))
        return run

    return register


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
            quiet=True,
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
        quiet=True,
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
        for library in data.get("psyq", {}).get("libraries", {}).values():
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
        root / "toolchains" / "rizin" / "bin" / "rizin",
    )
    missing = [str(path.relative_to(root)) for path in required if not path.exists()]
    if missing:
        raise FileNotFoundError("missing staged files: " + ", ".join(missing))
    _run([str(root / "bin" / "cc"), "-x", "c", "-E", "-"], cwd=root, quiet=True)
    for tool in REQUIRED_TOOLS:
        _run([str(root / tool), "--version"], cwd=root, quiet=True)
    cue, tracks = find_disc_set(root / "inputs" / "external")
    if not tracks or not all(track.is_file() for track in tracks):
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


@setup_task("submodules")
def _submodules(state: SetupState) -> str:
    _run(["git", "submodule", "update", "--init", "--recursive"], cwd=state.root, quiet=True)
    return "ready"


@setup_task("disc media")
def _disc(state: SetupState) -> str:
    disc = DiscToolchain(state.root)
    detail = disc.run(force=state.args.force)
    state.cue = disc.cue_path
    return detail


@setup_task("toolchain")
def _toolchain(state: SetupState) -> str:
    for toolchain in (
        Psn00bToolchain(state.layout),
        GccToolchain(state.layout),
        MaspsxToolchain(state.root),
        RizinToolchain(state.layout),
        M2cToolchain(state.root),
        AsmDifferToolchain(state.root),
        DecompPermuterToolchain(state.root),
        PsyqSignaturesToolchain(state.root),
    ):
        toolchain.run(force=state.args.force)
    return "PSn00b, GCC 2.7.2, maspsx, Rizin, m2c, asm-differ, decomp-permuter"


@setup_task("PsyQ 4.7")
def _psyq(state: SetupState) -> str:
    return PsyqToolchain(
        state.layout, archive=state.args.psyq_archive, archive_url=state.args.psyq_url
    ).run(force=state.args.force)


@setup_task("local tools")
def _tools(state: SetupState) -> str:
    _build_local_tools(state.root)
    return "bof3-disk, emi-ex"


@setup_task("compile commands")
def _compile_commands(state: SetupState) -> str:
    with contextlib.redirect_stdout(io.StringIO()):
        write_compile_commands(argparse.Namespace(root=state.root))
    return "compile_commands.json"


@setup_task("target images")
def _images(state: SetupState) -> str:
    if state.cue is None:
        raise RuntimeError("disc media task did not provide a CUE file")
    _extract_and_materialize(state.root, state.cue, force=state.args.force)
    return f"{len(load_target_manifests(state.root))} images"


@setup_task("verification")
def _verification(state: SetupState) -> str:
    verify_setup(state.root)
    return "ready"


def _render(status: str, label: str, detail: str) -> None:
    print(f"[{status}] {label:<{max(len(task.label) for task in TASKS)}}  {detail}")


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    state = SetupState(root=root, layout=repo_layout(root), args=args)
    for task in TASKS:
        try:
            _render("PASS", task.label, task.run(state))
        except (FileNotFoundError, RuntimeError, ValueError, tomllib.TOMLDecodeError) as exc:
            _render("FAIL", task.label, str(exc).replace("\n", "; "))
            raise
    print(f"setup: {len(TASKS)}/{len(TASKS)} tasks passed")
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
