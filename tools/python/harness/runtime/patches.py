"""PCSX-Redux patch target policy used by commands, setup and doctor."""

from __future__ import annotations

import json
from pathlib import Path

from ..io import file_sha256
from ..patches.git import git_text
from ..patches.models import Target
from ..patches.operations import run_operation
from ..patches.validation import describe_series, inspect_series

REDUX_BASE = "28438546c781fbe372a06399c82bed43ca2c6f4d"
SOURCE = "third_party/pcsx-redux"
GENERATED_LUAJIT = {
    "src/host/buildvm_arch.h",
    "src/jit/vmdef.lua",
    "src/lj_bcdef.h",
    "src/lj_ffdef.h",
    "src/lj_folddef.h",
    "src/lj_libdef.h",
    "src/lj_recdef.h",
    "src/lj_vm.S",
    "src/luajit.h",
}


def is_redux_artifact(submodule: str, relative: str, path: Path) -> bool:
    """Allow named native products; source/include trees have no suffix bypass."""
    if relative == submodule:
        if str(path) in {"pcsx-redux", "bins/Release/pcsx-redux"}:
            return True
        return (
            len(path.parts) >= 3
            and path.parts[1] == "Release"
            and (
                (path.parts[0] == "objs" and path.suffix == ".o")
                or (path.parts[0] == "deps" and path.suffix == ".dep")
            )
        )
    if relative == f"{submodule}/third_party/luajit":
        return str(path) in {
            "src/host/buildvm",
            "src/host/buildvm.o",
            "src/host/buildvm_asm.o",
            "src/host/buildvm_fold.o",
            "src/host/buildvm_lib.o",
            "src/host/buildvm_peobj.o",
            "src/host/minilua",
            "src/host/minilua.o",
            "src/libluajit.a",
            "src/lj_vm.o",
            "src/ljamalg.o",
            "src/luajit",
            "src/luajit.o",
            "src/luajit_relver.txt",
        }
    return False


def inspect_redux_sources(
    layout, paths: list[str], patched_files: set[str]
) -> dict[str, str]:
    """Reject unknown source/include files, ignored injections and nested edits."""
    generated = {}
    for relative in sorted(set(paths) | {SOURCE}):
        if relative != SOURCE and git_text(
            layout.root,
            "-C",
            relative,
            "diff",
            "--name-only",
            "--ignore-submodules=none",
            "HEAD",
            "--",
        ):
            raise ValueError(
                f"PCSX-Redux nested source has tracked changes: {relative}"
            )
        names = git_text(
            layout.root, "-C", relative, "ls-files", "--others", "-z"
        ).split("\0")
        for name in filter(None, names):
            within = str(Path(relative).relative_to(SOURCE) / name)
            if within in patched_files:
                continue  # The generic validator checks every added file's exact bytes.
            if relative == f"{SOURCE}/third_party/luajit" and name in GENERATED_LUAJIT:
                full = layout.root / relative / name
                if full.resolve() != full.absolute() or not full.is_file():
                    raise ValueError(
                        f"PCSX-Redux generated source is not regular: {full}"
                    )
                generated[f"{relative}/{name}"] = file_sha256(full)
            elif not is_redux_artifact(SOURCE, relative, Path(name)):
                raise ValueError(
                    f"PCSX-Redux has an untracked build input: {relative}/{name}"
                )
    return generated


def validate_redux_generation(layout, generated: dict[str, str]) -> None:
    """Reuse generated source bytes only from the preceding successful build."""
    if not generated:
        return
    receipt = layout.out_dir / "setup/pcsx-redux.json"
    try:
        previous = json.loads(receipt.read_text())
        expected = previous["identity"]["generated_sources"]
    except (OSError, ValueError, KeyError, TypeError) as error:
        raise ValueError(
            "PCSX-Redux generated sources lack a prior build receipt; preserve them before regeneration"
        ) from error
    if previous.get("schema") != "bof3.redux-build/v1" or generated != expected:
        raise ValueError(
            "PCSX-Redux generated sources changed; setup will not bless or overwrite them"
        )


def create_target(layout) -> Target:
    fields = git_text(layout.root, "ls-files", "--stage", "--", SOURCE).split()
    if (
        len(fields) != 4
        or fields[0] != "160000"
        or fields[2] != "0"
        or fields[1] != REDUX_BASE
    ):
        raise ValueError("PCSX-Redux requires its registered, approved base gitlink")

    def inspect(files):
        rows = git_text(layout.root, "submodule", "status", "--recursive", "--", SOURCE)
        if not rows or any(
            row.lstrip().startswith(("-", "+", "U")) for row in rows.splitlines()
        ):
            raise ValueError(
                "PCSX-Redux recursive revisions differ from their gitlinks"
            )
        generated = inspect_redux_sources(
            layout, [row.split()[1] for row in rows.splitlines()], files
        )
        return {
            "recursive_revisions": rows.splitlines(),
            "generated_sources": generated,
        }

    return Target(
        "pcsx-redux",
        layout.root,
        layout.root / SOURCE,
        REDUX_BASE,
        inspect,
        lambda evidence: validate_redux_generation(
            layout, evidence["generated_sources"]
        ),
        patch_order=(
            "debugger.patch",
            "exceptions.patch",
            "history.patch",
            "sound.patch",
            "video.patch",
            "cdrom.patch",
        ),
    )


def inspect_redux_target(layout, *, allow_pristine: bool = False) -> dict:
    report = describe_series(inspect_series(create_target(layout)))
    if not allow_pristine and not all(patch["applied"] for patch in report["patches"]):
        raise ValueError("PCSX-Redux customization is not applied; run setup")
    return report


def prepare_redux_target(layout) -> dict:
    return run_operation(create_target(layout), "apply")["identity"]
