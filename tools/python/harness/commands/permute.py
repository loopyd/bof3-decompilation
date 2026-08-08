"""Prepare and run the repository's explicitly configured decomp-permuter."""

from __future__ import annotations

import argparse
import fcntl
import os
import subprocess
import sys
from pathlib import Path
import re

from ..toolchain.permuter import DecompPermuterToolchain
from ._common import add_example_argument, add_root_argument, run_main


def require_inside_root(path: Path, description: str, root: Path) -> None:
    try:
        path.relative_to(root)
    except ValueError as exc:
        raise ValueError(f"{description} must be inside {root}: {path}") from exc


def resolve_function_name(source: Path, explicit: str | None, root: Path) -> str:
    """Return the compiled symbol name for the permuter workspace.

    An explicit name wins; otherwise the owning target map names the
    compiled symbol (map/Splat agreement, never the filename stem and never
    a synthesized ``func_<ADDR>``).
    """

    if explicit is not None:
        function = explicit
    else:
        from ..domain.sources import compiled_symbol_name, source_address

        address = source_address(source)
        function = compiled_symbol_name(root, source, address)
    if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", function):
        raise ValueError(f"invalid function name: {function!r}")
    return function


def default_directory(source: Path, root: Path) -> Path:
    relative_source = source.relative_to(root).with_suffix("")
    directory = (root / "out" / "permuter" / relative_source).resolve()
    require_inside_root(directory, "default permuter directory", root)
    return directory


def assembly_path(source: Path, function: str, root: Path) -> Path:
    target_relative = source.parent.relative_to(root / "src")
    directory = root / "out" / "splat" / target_relative / "asm"
    exact = directory / f"{function}.s"
    if exact.is_file():
        return exact
    match = re.fullmatch(r"func_([0-9a-fA-F]{8})", function)
    if match is not None:
        upper = directory / f"func_{match.group(1).upper()}.s"
        if upper.is_file():
            return upper
    return exact


def ensure_target_assembly(
    source: Path, function: str, directory: Path, root: Path
) -> None:
    target = directory / "target.s"
    if target.is_file():
        require_inside_root(target.resolve(), "target.s", root)
        return
    if target.exists() or target.is_symlink():
        raise ValueError(f"target.s is not a regular file: {target}")

    canonical = assembly_path(source, function, root)
    if canonical.is_file():
        resolved_candidate = canonical.resolve()
        require_inside_root(resolved_candidate, "assembly artifact", root)
        directory.mkdir(parents=True, exist_ok=True)
        target.write_bytes(resolved_candidate.read_bytes())
        return

    source_relative = source.relative_to(root).as_posix()
    raise FileNotFoundError(
        f"no original assembly found for {function}; looked for:\n"
        f"  {canonical}\n"
        "run the Splat split workflow before starting the permuter, then retry "
        f"`bin/permute {source_relative}`"
    )


def validate(args: argparse.Namespace, root: Path) -> tuple[Path, str, Path, Path]:
    source = args.source.expanduser().resolve()
    if not source.is_file():
        raise FileNotFoundError(f"source does not exist: {source}")
    require_inside_root(source, "source", root)
    try:
        source.relative_to(root / "src")
    except ValueError as exc:
        raise ValueError(f"source must be inside {root / 'src'}: {source}") from exc

    function = resolve_function_name(source, args.function, root)
    directory = (
        default_directory(source, root)
        if args.directory is None
        else args.directory.expanduser().resolve()
    )
    require_inside_root(directory, "permuter directory", root)
    if args.directory is not None and not directory.is_dir():
        raise FileNotFoundError(f"permuter directory does not exist: {directory}")
    if directory.exists() and not directory.is_dir():
        raise ValueError(f"permuter directory is not a directory: {directory}")
    preparer = root / "tools" / "prep-permuter.py"
    if not preparer.is_file():
        raise FileNotFoundError(
            f"decomp-permuter workflow is not wired: expected {preparer}"
        )
    return source, function, directory, preparer


def permuter_arguments(args: argparse.Namespace) -> list[str]:
    options: list[str] = []
    for name, option in (
        ("show_errors", "--show-errors"),
        ("show_timings", "--show-timings"),
        ("print_diffs", "--print-diffs"),
        ("abort_exceptions", "--abort-exceptions"),
        ("better_only", "--better-only"),
        ("best_only", "--best-only"),
        ("stop_on_zero", "--stop-on-zero"),
        ("quiet", "--quiet"),
        ("stack_diffs", "--stack-diffs"),
        ("no_context_output", "--no-context-output"),
        ("no_ignore_branch_targets", "--no-ignore-branch-targets"),
        ("debug", "--debug"),
    ):
        if getattr(args, name):
            options.append(option)
    for name, option in (
        ("algorithm", "--algorithm"),
        ("keep_prob", "--keep-prob"),
        ("only_if_below", "--only-if-below"),
        ("speed", "--speed"),
        ("seed", "--seed"),
    ):
        value = getattr(args, name)
        if value is not None:
            options.extend((option, str(value)))
    if args.jobs is not None:
        options.extend(("-j", str(args.jobs)))
    return options


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    source, function, directory, preparer = validate(args, root)
    if args.jobs is not None and args.jobs < 0:
        raise ValueError("--jobs must not be negative")
    if args.time_limit is not None and args.time_limit <= 0:
        raise ValueError("--time-limit must be positive")
    if (
        args.time_limit is not None
        and args.time_limit > 60
        and not args.allow_long_run
    ):
        raise ValueError(
            "--time-limit is capped at 60s per run (matching rule); "
            "pass --allow-long-run for interactive search only"
        )
    if args.prepare_only and args.prepared:
        raise ValueError("--prepare-only cannot be combined with --prepared")

    directory.mkdir(parents=True, exist_ok=True)
    lock_path = directory / ".coordinator.lock"
    acquired = False
    lock = lock_path.open("w", encoding="ascii")
    try:
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
            acquired = True
        except BlockingIOError as exc:
            raise RuntimeError(
                f"another decomp-permuter coordinator is already running for "
                f"{source.relative_to(root)}"
            ) from exc
        lock.write(f"pid={os.getpid()} source={source.relative_to(root)}\n")
        lock.flush()

        if not args.prepared:
            ensure_target_assembly(source, function, directory, root)
            toolchain = DecompPermuterToolchain(root)
            prepare = subprocess.run(
                [
                    str(toolchain.python),
                    str(preparer),
                    str(source),
                    function,
                    str(directory),
                ],
                cwd=root,
                check=False,
            )
            if prepare.returncode != 0:
                return prepare.returncode
        else:
            required = ("base.c", "target.o", "compile.sh", "settings.toml")
            missing = [name for name in required if not (directory / name).is_file()]
            if missing:
                raise FileNotFoundError(
                    f"prepared workspace is missing: {', '.join(missing)}"
                )
        if args.prepare_only:
            return 0
        toolchain = DecompPermuterToolchain(root)
        result = toolchain.execute(
            [*permuter_arguments(args), str(directory)],
            timeout=args.time_limit,
        )
        return result.returncode
    except subprocess.TimeoutExpired:
        print(
            f"time limit reached after {args.time_limit:g}s; retained workspace {directory.relative_to(root)}",
            file=sys.stderr,
        )
        return 0
    finally:
        lock.close()
        if acquired:
            try:
                lock_path.unlink()
            except FileNotFoundError:
                pass


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Prepare and optionally run decomp-permuter for one function.",
        prog="permute",
    )
    add_root_argument(parser)
    parser.add_argument(
        "source",
        type=Path,
        nargs="?",
        help="TARGET@0xADDRESS (preferred) or an existing lift source path",
    )
    add_example_argument(parser, "bin/permute exe/logo@0x801CE758 --time-limit 30")
    parser.add_argument(
        "function",
        nargs="?",
        help="function name; defaults to the compiled map symbol",
    )
    parser.add_argument(
        "directory",
        nargs="?",
        type=Path,
        help="existing permuter directory; defaults under out/permuter/",
    )
    parser.add_argument("--prepare-only", action="store_true")
    parser.add_argument(
        "--prepared",
        action="store_true",
        help="run the existing workspace without regenerating base.c or target.o",
    )
    parser.add_argument("--show-errors", action="store_true")
    parser.add_argument("--show-timings", action="store_true")
    parser.add_argument("--print-diffs", action="store_true")
    parser.add_argument("--abort-exceptions", action="store_true")
    parser.add_argument("--better-only", action="store_true")
    parser.add_argument("--best-only", dest="best_only", action="store_true")
    parser.add_argument(
        "--all-improvements",
        dest="best_only",
        action="store_false",
        help="report every candidate no worse than the base",
    )
    parser.add_argument(
        "--stop-on-zero",
        dest="stop_on_zero",
        action="store_true",
        help="stop as soon as a perfect (score 0) match is found (default)",
    )
    parser.add_argument(
        "--no-stop-on-zero",
        dest="stop_on_zero",
        action="store_false",
        help="keep running after a perfect match is found",
    )
    parser.add_argument(
        "--quiet", dest="quiet", action="store_true", help="hide iteration progress"
    )
    parser.add_argument(
        "--verbose",
        dest="quiet",
        action="store_false",
        help="show decomp-permuter iteration progress",
    )
    parser.add_argument("--stack-diffs", action="store_true")
    parser.add_argument("--no-context-output", action="store_true")
    parser.add_argument("--no-ignore-branch-targets", action="store_true")
    parser.add_argument("--algorithm", choices=("difflib", "levenshtein"))
    parser.add_argument("--keep-prob")
    parser.add_argument("--only-if-below", type=int)
    parser.add_argument("--speed", type=int, nargs="?", const=100)
    parser.add_argument("--seed")
    parser.add_argument("-j", "--jobs", type=int)
    parser.add_argument(
        "--time-limit",
        type=float,
        default=30.0,
        metavar="SECONDS",
        help="stop this coordinator after SECONDS (default 30, capped at 60; the upstream permuter has no native timer)",
    )
    parser.add_argument(
        "--allow-long-run",
        action="store_true",
        help="override the 60s --time-limit cap (interactive search only)",
    )
    parser.add_argument("--debug", action="store_true")
    parser.set_defaults(
        quiet=False, best_only=True, stop_on_zero=True, handler=_resolve_and_run
    )
    return parser


def _resolve_and_run(args: argparse.Namespace) -> int:
    if args.source is None:
        raise ValueError("TARGET@0xADDRESS is required")
    raw_source = str(args.source)
    if "@" in raw_source:
        from harness.commands._lift_m2c import resolve_function

        function_id, _, args.source = resolve_function(raw_source)
        if args.source is None:
            raise FileNotFoundError(
                f"lifted source does not exist for {function_id.target.value}@0x{function_id.address:08X}"
            )
        # The permuter function name is resolved from the owning map in
        # validate(); never synthesize func_<ADDR> here.
    return run(args)


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
