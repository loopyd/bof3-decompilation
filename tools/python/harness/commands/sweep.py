"""``bin/harness lift sweep``: measure many clean-C candidate shapes in one call.

For each variant it installs the file at SOURCE_PATH, measures the native asm diff,
and prints one table row (variant, insns matched, bytes, first difference).  The
original file content is restored on exit, and the best variant (highest matched
insns, then fewest bytes) is left installed when the caller passes ``--keep-best``.

Cost contract: spawning ``bin/harness lift asm-diff`` per variant re-pays the whole
per-invocation pipeline (CMake configure check, glob verification, the pinned
inventory edge and the ninja graph) and, under concurrent lanes, contends on the
shared build tree; a measured variant cost 10.5 s idle and ~60 s under fan-out, so
an unbounded variant loop burns a lane.  The default path therefore resolves the
comparison context and the canonical compile command **once**, compiles each variant
directly with that command (the same mechanism ``lift flag-search`` uses) and runs
the link/byte-match step in-process.  The subprocess route remains as a fallback
whenever resolution or compilation is unavailable, so behaviour never regresses.
"""

from __future__ import annotations

import argparse
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from harness.common.cli import add_example_argument, run_main
from harness.io import repo_layout

USAGE = (
    "usage: bin/harness lift sweep SELECTOR SOURCE_PATH "
    "[--keep-best] [--subprocess] VARIANT1.c [VARIANT2.c ...]"
)
EXAMPLE = "bin/harness lift sweep exe/logo@0x801CE758 src/x.c a.c b.c"

_LINE_RE = re.compile(r"^(?:MATCH|DIFF)")
_INSNS_RE = re.compile(r".*insn=([0-9]+)/([0-9]+).*")
_BYTES_RE = re.compile(r".*bytes=([0-9]+)->([0-9]+).*")
_FIRST_RE = re.compile(r".*first=([^ ]+).*")


def _measure(selector: str) -> tuple[str, str, str]:
    command = [
        str(repo_layout().root / "bin" / "harness"),
        "lift",
        "asm-diff",
        selector,
        "--detail",
        "normal",
    ]
    result = subprocess.run(command, check=False, capture_output=True, text=True)
    lines = [
        line
        for line in (result.stdout + result.stderr).splitlines()
        if _LINE_RE.match(line)
    ]
    line = lines[-1] if lines else ""
    insns = _INSNS_RE.match(line)
    bytes_match = _BYTES_RE.match(line)
    first = _FIRST_RE.match(line)
    return (
        f"{insns.group(1)}/{insns.group(2)}" if insns else "-",
        f"{bytes_match.group(1)}->{bytes_match.group(2)}" if bytes_match else "-",
        first.group(1) if first else "-",
    )


class _FastContext:
    """One resolved comparison context plus its compile command sequence."""

    __slots__ = ("repo", "request", "resolved", "commands", "directory")

    def __init__(self, repo, request, resolved, commands, directory) -> None:
        self.repo = repo
        self.request = request
        self.resolved = resolved
        self.commands = commands
        self.directory = directory


def _ninja_compile_commands(
    repo, object_path: Path, source: Path
) -> tuple[list[list[str]], str] | None:
    """Return ninja's own compile command for one object without running the graph.

    ``ninja -t commands`` prints the recipe ninja would execute; it does not run
    CMake, re-verify globs, refresh the pinned inventory or walk the graph, so it
    gives the exact compiler invocation for the price of a ninja lookup.
    """

    build_tree = Path(repo.root) / "build" / "cmake"
    if not (build_tree / "build.ninja").is_file():
        return None
    # Ninja reports object targets by their absolute path in this generated tree.
    target = str(object_path.resolve())
    result = subprocess.run(
        ["ninja", "-C", str(build_tree), "-t", "commands", target],
        check=False,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        relative = os.path.relpath(object_path, build_tree)
        result = subprocess.run(
            ["ninja", "-C", str(build_tree), "-t", "commands", relative],
            check=False,
            capture_output=True,
            text=True,
        )
    if result.returncode != 0:
        return None
    for line in result.stdout.splitlines():
        if " -c " not in line or source.name not in line:
            continue
        # Ninja recipes here are ``cd DIR && mkdir ... && <compiler> ...``.  Run the
        # segments directly (no shell) so the measurement costs one compiler
        # invocation instead of the whole per-invocation pipeline.
        working = str(repo.root)
        body = line
        match = re.match(r"cd\s+(\S+)\s+&&\s+(.*)$", line)
        if match:
            working = match.group(1)
            body = match.group(2)
        segments = [segment.strip() for segment in body.split(" && ") if segment.strip()]
        commands: list[list[str]] = []
        for segment in segments:
            if any(character in segment for character in "|;<>$`*"):
                return None
            commands.append(shlex.split(segment))
        if not any("-c" in command for command in commands):
            return None
        return commands, working
    return None


def _prepare_fast(source: Path) -> _FastContext | None:
    """Resolve the comparison and compile command once, or return None.

    Any missing input (no compile database entry, unresolved selector, no
    ownership) simply declines the fast path; the caller falls back to the
    subprocess route rather than reporting a new failure mode.
    """

    try:
        from harness.match._asm_diff_payload import AsmDiffRequest
        from harness.match._asm_diff_run import _asm_diff_resolve
        from harness.match.flag_search import _compile_command

        repo = repo_layout()
        request = AsmDiffRequest(source_path=source)
        resolved = _asm_diff_resolve(repo, request)
        command_and_cwd = _ninja_compile_commands(repo, resolved["object_path"], source)
        if command_and_cwd is None:
            single, directory = _compile_command(repo, source)
            commands, directory = [single], str(directory)
        else:
            commands, directory = command_and_cwd
    except Exception:
        return None
    if not commands:
        return None
    return _FastContext(repo, request, resolved, commands, directory)


def _measure_compiled(context: _FastContext) -> tuple[str, str, str]:
    """Compile the installed variant with the canonical command and compare."""

    from harness.match._asm_diff_run import _asm_diff_compare

    for command in context.commands:
        compile_result = subprocess.run(
            command,
            cwd=context.directory,
            check=False,
            capture_output=True,
            text=True,
        )
        if compile_result.returncode != 0:
            raise RuntimeError(
                (compile_result.stdout + compile_result.stderr).strip()[-400:]
            )
    payload = _asm_diff_compare(context.repo, context.request, context.resolved)
    instructions = payload.get("instruction_count") or {}
    matching = instructions.get("matching")
    original = instructions.get("original")
    insns = f"{matching}/{original}" if matching is not None else "-"
    current = payload.get("current_size")
    bytes_text = (
        f"{payload.get('original_size')}->{current}"
        if current is not None
        else "-"
    )
    return insns, bytes_text, payload.get("first_mismatch") or "-"


def _run(args: argparse.Namespace) -> int:
    selector = args.selector
    source = args.source
    variants = list(args.variants)
    keep_best = args.keep_best
    if not source.is_file():
        print(f"shape-sweep: source path does not exist: {source}", file=sys.stderr)
        return 2

    context = None if args.subprocess else _prepare_fast(source)
    route = "in-process" if context is not None else "subprocess"
    print(f"shape-sweep: {len(variants)} variant(s) via {route} route")

    best_file: Path | None = None
    best_insn = -1
    backup: Path | None = None
    try:
        descriptor, name = tempfile.mkstemp()
        os.close(descriptor)
        backup = Path(name)
        shutil.copyfile(source, backup)
        print(f"{'VARIANT':<28} {'INSNS':>8} {'BYTES':>8} FIRST")
        for variant_name in variants:
            variant = Path(variant_name)
            if not variant.is_file():
                print(f"{variant.name:<28} {'-':>8} {'-':>8} missing")
                continue
            shutil.copyfile(variant, source)
            if context is not None:
                try:
                    insns, bytes_text, first = _measure_compiled(context)
                except Exception as error:
                    print(f"  fast route declined: {error}")
                    context = None
                    insns, bytes_text, first = _measure(selector)
            else:
                insns, bytes_text, first = _measure(selector)
            print(f"{variant.name:<28} {insns:>8} {bytes_text:>8} {first}")
            matched = insns.split("/", 1)[0]
            if not matched.isdigit():
                matched = "0"
            if int(matched) > best_insn:
                best_insn = int(matched)
                best_file = variant
        return 0
    finally:
        if keep_best and best_file is not None:
            shutil.copyfile(best_file, source)
            print(f"shape-sweep: kept best variant {best_file.name}")
        elif backup is not None:
            shutil.copyfile(backup, source)
        if backup is not None:
            backup.unlink(missing_ok=True)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness lift sweep", description=__doc__)
    parser.add_argument("selector", help="TARGET@0xADDRESS")
    parser.add_argument("source", type=Path, help="source file the variants replace")
    parser.add_argument("variants", nargs="+", help="candidate clean-C files")
    parser.add_argument(
        "--keep-best", action="store_true", help="leave the best variant installed"
    )
    parser.add_argument(
        "--subprocess",
        action="store_true",
        help="force the legacy per-variant subprocess measurement route",
    )
    add_example_argument(parser, EXAMPLE)
    parser.set_defaults(handler=_run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)
