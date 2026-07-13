"""Prepare and run decomp-permuter workspaces without mutating tracked source."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys
import time
from typing import Any

from ..domain import load_target_manifests, normalize_target_id, parse_address
from ..match._asm_resolve import compile_command_for_source
from ..match._asm_resolve import (
    default_binary_for_source,
    extract_original_bytes,
    infer_original_size,
    overlay_load_address_for_source,
)
from ..paths import repo_layout


def _repair_psyq_register_parameters(source: str) -> str:
    """Undo asm.h's `fp` macro only where it corrupts pointer parameters."""

    return re.sub(r"([*&]\s*)\$30\b", r"\1fp", source)


def _original_target_assembly(function_name: str, original: bytes) -> str:
    if len(original) % 4:
        raise ValueError("MIPS function byte length must be word-aligned")
    words = [
        f"    .word 0x{int.from_bytes(original[i : i + 4], 'little'):08x}"
        for i in range(0, len(original), 4)
    ]
    return (
        ".section .text\n"
        f".globl {function_name}\n.type {function_name}, @function\n"
        f"{function_name}:\n" + "\n".join(words) + "\n"
        f".size {function_name}, .-{function_name}\n"
    )


def _resolve_source(root: Path, source_or_function: str) -> tuple[str, Path, int]:
    if "@" in source_or_function:
        target, address_text = source_or_function.rsplit("@", 1)
        address = parse_address(address_text)
        target_id = normalize_target_id(target)
        source = (
            root
            / load_target_manifests(root)[target_id.value].source_dir
            / f"func_{address:08x}.c"
        )
        return target_id.value, source, address
    source = Path(source_or_function).expanduser().resolve()
    if not source.is_file():
        raise FileNotFoundError(f"source not found: {source}")
    stem = source.stem.removeprefix("func_")
    manifests = load_target_manifests(root)
    source_dir = source.parent.relative_to(root).as_posix()
    target_id = next(
        (
            target
            for target, manifest in manifests.items()
            if manifest.source_dir == source_dir
        ),
        source.parent.name,
    )
    return target_id, source, parse_address(stem)


def prepare_permuter(
    root: Path, source_or_function: str, work_root: Path | None = None
) -> dict[str, Any]:
    root = root.resolve()
    target_id, source, address = _resolve_source(root, source_or_function)
    if not source.is_file():
        raise FileNotFoundError(f"lifted source not found: {source}")
    output = (work_root or root / "out" / "matching").resolve()
    bundle = output / target_id / f"func_{address:08x}" / "permuter"
    bundle.mkdir(parents=True, exist_ok=True)
    base_source = bundle / "base.c"
    preprocess = subprocess.run(
        [
            "cpp",
            "-P",
            "-I",
            str(source.parent),
            "-I",
            str(root / "include"),
            "-I",
            str(root / "toolchains" / "psyq" / "4.7" / "include"),
            str(source),
        ],
        cwd=root,
        text=True,
        capture_output=True,
        check=False,
    )
    if preprocess.returncode != 0:
        raise RuntimeError(
            f"failed to preprocess {source} for permuter: {preprocess.stderr.strip()}"
        )
    # PsyQ asm.h defines `fp` as register `$30`, including inside prototypes
    # from libcd.h.  Keep asm strings intact while repairing pointer parameters
    # which pycparser otherwise cannot tokenize.
    preprocessed = _repair_psyq_register_parameters(preprocess.stdout)
    base_source.write_text(preprocessed, encoding="utf-8")

    function_name = f"func_{address:08x}"
    stripped = subprocess.run(
        [
            sys.executable,
            str(root / "third_party" / "decomp-permuter" / "strip_other_fns.py"),
            str(base_source),
            function_name,
        ],
        cwd=root,
        text=True,
        capture_output=True,
        check=False,
    )
    if stripped.returncode != 0:
        raise RuntimeError(
            f"failed to isolate {function_name} for permuter: {stripped.stderr.strip()}"
        )

    layout = repo_layout(root)
    binary_path = default_binary_for_source(layout, source)
    load_address = overlay_load_address_for_source(layout, source)
    size = infer_original_size(
        source,
        address=address,
        binary_path=binary_path,
        load_address=load_address,
    )
    original = extract_original_bytes(
        binary_path, address=address, size=size, load_address=load_address
    )
    target_asm = bundle / "target.s"
    target_asm.write_text(
        _original_target_assembly(function_name, original),
        encoding="ascii",
    )
    assembler = layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-as"
    assembled = subprocess.run(
        [
            str(assembler),
            "-EL",
            "-march=r3000",
            "-mips1",
            str(target_asm),
            "-o",
            str(bundle / "target.o"),
        ],
        cwd=root,
        text=True,
        capture_output=True,
        check=False,
    )
    if assembled.returncode != 0:
        raise RuntimeError(
            f"failed to assemble permuter target: {assembled.stderr.strip()}"
        )

    compile_script = (
        '#!/bin/sh\nset -eu\nBUNDLE="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"\n'
    )
    try:
        command = compile_command_for_source(repo_layout(root), source)
    except (FileNotFoundError, ValueError):
        command = None
    if command is not None:
        raw_command = command.get("command")
        if raw_command:
            argv = shlex.split(raw_command)
            output = command["output"]
            argv = [
                (
                    '"$INPUT"'
                    if Path(arg).resolve() == source
                    else (
                        '"$OUTPUT"'
                        if arg == output
                        or Path(arg).resolve()
                        == (Path(command["directory"]) / output).resolve()
                        else shlex.quote(arg)
                    )
                )
                for arg in argv
            ]
            compile_script += 'test "${2:-}" = "-o"\n'
            compile_script += (
                'case "$1" in /*) INPUT="$1" ;; *) INPUT="$PWD/$1" ;; esac\n'
            )
            compile_script += (
                'case "$3" in /*) OUTPUT="$3" ;; *) OUTPUT="$PWD/$3" ;; esac\n'
            )
            # decomp-permuter reserves the output path before invoking the
            # compiler; the PsyQ driver refuses to replace that empty file.
            compile_script += 'rm -f -- "$OUTPUT"\n'
            compile_script += f"cd {shlex.quote(command['directory'])}\n"
            compile_script += f"exec {' '.join(argv)}\n"
        else:
            compile_script += "# compile_commands.json has no shell command.\n"
    else:
        compile_script += "# Build metadata is unavailable; run `just build` first.\n"
    (bundle / "compile.sh").write_text(compile_script, encoding="utf-8")
    (bundle / "compile.sh").chmod(0o755)
    objdump = root / "bin" / "objdump"
    (bundle / "settings.toml").write_text(
        'algorithm = "levenshtein"\nbetter_only = true\nbest_only = true\n'
        "stop_on_zero = true\nstack_diffs = true\nno_context_output = true\n"
        f'func_name = "{function_name}"\n'
        'compiler_type = "gcc"\n'
        f'objdump_command = "{objdump} -drz"\n',
        encoding="utf-8",
    )
    metadata = {
        "schema": "harness.permuter/v1",
        "target": target_id,
        "function": f"{target_id}@{address:08x}",
        "source": str(source.relative_to(root)),
        "source_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
        "bundle": (
            str(bundle.relative_to(root))
            if bundle.is_relative_to(root)
            else str(bundle)
        ),
        "target_object": (
            str((bundle / "target.o").relative_to(root))
            if (bundle / "target.o").is_file()
            else None
        ),
        "status": "ready",
    }
    (bundle / "metadata.json").write_text(
        json.dumps(metadata, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    return metadata


def run_permuter(
    root: Path,
    metadata: dict[str, Any],
    *,
    jobs: int | None = None,
    verbose: bool = False,
    show_errors: bool = False,
    show_timings: bool = False,
) -> dict[str, Any]:
    """Run the pinned permuter and retain only strict score improvements."""

    root = root.resolve()
    bundle = Path(metadata["bundle"])
    if not bundle.is_absolute():
        bundle = root / bundle
    target_object = bundle / "target.o"
    if not target_object.is_file():
        raise RuntimeError(
            f"permuter target object is missing: {target_object}; run `just build` first"
        )
    tool = root / "third_party" / "decomp-permuter" / "permuter.py"
    if not tool.is_file():
        raise RuntimeError(f"decomp-permuter is missing: {tool}")
    command = [
        sys.executable,
        str(tool),
        str(bundle),
        "--better-only",
        "--best-only",
        "--stop-on-zero",
        "--stack-diffs",
        "--no-context-output",
        "--algorithm",
        "levenshtein",
        "-j",
        str(jobs or 1),
    ]
    if show_errors:
        command.append("--show-errors")
    if show_timings:
        command.append("--show-timings")
    started = time.perf_counter()
    completed = subprocess.run(
        command,
        cwd=root,
        text=True,
        capture_output=not verbose,
        check=False,
    )
    elapsed = time.perf_counter() - started
    if not verbose:
        (bundle / "permuter.stdout").write_text(
            completed.stdout or "", encoding="utf-8"
        )
        (bundle / "permuter.stderr").write_text(
            completed.stderr or "", encoding="utf-8"
        )
    if completed.returncode != 0:
        raise RuntimeError(
            f"decomp-permuter failed with exit code {completed.returncode}; see {bundle}"
        )

    candidates = bundle / "candidates"
    candidates.mkdir(exist_ok=True)
    retained: list[dict[str, Any]] = []
    for output in sorted(bundle.glob("output-*/")):
        source = output / "source.c"
        score_path = output / "score.txt"
        if not source.is_file() or not score_path.is_file():
            continue
        try:
            score = int(score_path.read_text(encoding="utf-8").strip())
        except ValueError:
            continue
        if retained and score >= retained[-1]["score"]:
            continue
        candidate_dir = candidates / f"{len(retained) + 1:04d}"
        candidate_dir.mkdir(exist_ok=True)
        shutil.copyfile(source, candidate_dir / "source.c")
        (candidate_dir / "score.txt").write_text(f"{score}\n", encoding="utf-8")
        candidate = {
            "path": str((candidate_dir / "source.c").relative_to(root)),
            "score": score,
            "parent_score": retained[-1]["score"] if retained else None,
        }
        (candidate_dir / "metadata.json").write_text(
            json.dumps(
                {
                    "schema": "harness.permuter-candidate/v1",
                    "function": metadata["function"],
                    "seed": metadata.get("seed"),
                    "elapsed_seconds": round(elapsed, 3),
                    **candidate,
                },
                indent=2,
                sort_keys=True,
            )
            + "\n",
            encoding="utf-8",
        )
        retained.append(candidate)
    if retained:
        best = candidates / "best"
        if best.exists() or best.is_symlink():
            best.unlink()
        best.symlink_to(Path(retained[-1]["path"]).parent.name)
    result = {
        "schema": "harness.permuter-run/v1",
        "bundle": metadata["bundle"],
        "function": metadata["function"],
        "improvements": len(retained),
        "best": retained[-1] if retained else None,
        "elapsed_seconds": round(elapsed, 3),
        "show_errors": show_errors,
        "show_timings": show_timings,
        "status": "improved" if retained else "no-improvement",
    }
    (bundle / "result.json").write_text(
        json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    return result
