import re

from ..io import repo_layout
from ..toolchain.permuter import DecompPermuterToolchain

def require_inside_root(path: Path, description: str, root: Path) -> None:
    try:
        path.relative_to(root)
    except ValueError as exc:
        raise ValueError(f"{description} must be inside {root}: {path}") from exc
def default_directory(source: Path, root: Path) -> Path:
    relative_source = source.relative_to(root).with_suffix("")
    directory = (root / "out" / "permuter" / relative_source).resolve()
    require_inside_root(directory, "default permuter directory", root)
    return directory


def assembly_path(source: Path, function: str, root: Path) -> Path:
    target_relative = source.parent.relative_to(root / "src")
    directory = root / "out" / "splat" / target_relative / "asm"
def ensure_target_assembly(source: Path, function: str, directory: Path, root: Path) -> None:
    target = directory / "target.s"
    if target.is_file():
        require_inside_root(target.resolve(), "target.s", root)
    canonical = assembly_path(source, function, root)
        require_inside_root(resolved_candidate, "assembly artifact", root)
    source_relative = source.relative_to(root).as_posix()
def validate(args: argparse.Namespace, root: Path) -> tuple[Path, str, Path, Path]:
    require_inside_root(source, "source", root)
    try:
        source.relative_to(root / "src")
    except ValueError as exc:
        raise ValueError(f"source must be inside {root / 'src'}: {source}") from exc
        default_directory(source, root)
    require_inside_root(directory, "permuter directory", root)
    preparer = root / "tools" / "prep-permuter.py"
    if not preparer.is_file():
        raise FileNotFoundError(
            f"decomp-permuter workflow is not wired: expected {preparer}"
        )
    return source, function, directory, preparer
    root = args.root.resolve()
    source, function, directory, preparer = validate(args, root)
                f"{source.relative_to(root)}"
            ) from exc
        lock.write(f"pid={os.getpid()} source={source.relative_to(root)}\n")
        lock.flush()

        if not args.prepared:
            ensure_target_assembly(source, function, directory, root)
            toolchain = DecompPermuterToolchain(root)
            prepare = subprocess.run(
                [str(toolchain.python), str(preparer), str(source), function, str(directory)],
                cwd=root,
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
        description="Prepare and optionally run decomp-permuter for one function.",
        prog="permute",
    )
    args = build_parser().parse_args(arguments)
    try:
