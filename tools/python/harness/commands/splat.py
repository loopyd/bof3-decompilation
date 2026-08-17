"""Run Splat for exactly one manifest-owned target."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from ..domain import lookup_target_manifest
from ..domain.sources import LiftMetadataError, lift_metadata
from ..domain.layout import parse_splat_layout
from ..toolchain.splat import SplatToolchain
from ._common import add_root_argument, run_main


def _legacy_stub_candidates(root: Path, manifest) -> list[tuple[Path, Path, int]]:
    """Yield ``(legacy_stub_path, authored_owner_path, expected_address)`` for
    every Splat ``c`` boundary whose reviewed ``@source`` names a nested or
    out-of-root owner.

    Upstream Splat writes a flat ``source_dir/<boundary-name>.c`` stub for such
    boundaries, keyed by the Splat boundary name — never by the authored
    destination basename, which may legitimately be collision-renamed (for
    example ``advancePanelXTo320_game00_801996FC.c`` under boundary
    ``advancePanelXTo320``). The flat stub path is the only path Splat may
    (re)generate; the metadata-tagged owner path is never touched by Splat.
    """

    source_dir = root / manifest.source_dir
    splat_path = root / manifest.splat
    if not splat_path.is_file():
        return []
    layout = parse_splat_layout(splat_path, manifest.load_address)
    candidates: list[tuple[Path, Path, int]] = []
    for boundary in layout.boundaries:
        if boundary.kind != "c" or not boundary.source or not boundary.name:
            continue
        source = Path(boundary.source)
        if source.parts and source.parts[0] == "src":
            owner = root / source.with_suffix(".c")
        else:
            owner = source_dir / source.with_suffix(".c")
        try:
            nested = len(owner.relative_to(source_dir).parts) > 1
        except ValueError:
            nested = True  # out-of-root claimed owner
        if not nested:
            continue
        stub = source_dir / f"{Path(boundary.name).name}.c"
        candidates.append((stub, owner, boundary.virtual_start))
    return candidates


def _assert_pre_run_safety(root: Path, manifest) -> None:
    """Refuse any pre-existing legacy path unless it is our projected stub.

    A metadata-free file may still be authored, so content shape alone cannot
    authorize overwriting it. Only a byte-identical prior projection proves the
    path is disposable Splat output.
    """

    for stub, owner, _address in _legacy_stub_candidates(root, manifest):
        if not stub.is_file():
            continue
        projected = (
            root
            / "out"
            / "splat"
            / manifest.id.value
            / "source-view"
            / f"{owner.stem}.c"
        )
        if not projected.is_file() or stub.read_bytes() != projected.read_bytes():
            raise ValueError(
                "refusing Splat: pre-existing source would be overwritten at "
                f"{stub.relative_to(root)}"
            )


def _project_regenerated_stubs(root: Path, manifest) -> None:
    """Move every freshly Splat-regenerated metadata-free root stub into the
    ignored ``out/splat/<target>/source-view/`` projection.

    Idempotent and pre-run safe: the projection is refreshed atomically on
    every run (never skipped just because it already exists), and only files
    matching a Splat-generated stub (no ``@source`` metadata) for a boundary
    with an existing, address-matching authored owner are touched.  Authored
    sources are never deleted or overwritten; previously projected bytes are
    never lost (write-temp + replace).
    """

    splat_path = root / manifest.splat
    if not splat_path.is_file():
        return
    for stub, owner, expected_address in _legacy_stub_candidates(root, manifest):
        if not stub.is_file() or not owner.is_file():
            continue
        try:
            owner_address, _behavior = lift_metadata(owner)
        except (OSError, UnicodeError, LiftMetadataError):
            continue
        if owner_address != expected_address or "@source" in stub.read_text(
            encoding="utf-8", errors="ignore"
        ):
            continue
        projected = (
            root
            / "out"
            / "splat"
            / manifest.id.value
            / "source-view"
            / f"{owner.stem}.c"
        )
        projected.parent.mkdir(parents=True, exist_ok=True)
        temporary = projected.with_name(f".{projected.name}.tmp")
        temporary.write_text(stub.read_text(encoding="utf-8"), encoding="utf-8")
        temporary.replace(projected)
        stub.unlink()


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    manifest = lookup_target_manifest(root, args.target)
    if manifest is None:
        raise ValueError(f"unknown target: {args.target}")
    toolchain = SplatToolchain(root)
    if not toolchain.executable.is_file():
        raise FileNotFoundError(
            f"missing Splat executable: {toolchain.executable}; run just setup"
        )
    _assert_pre_run_safety(root, manifest)
    result = toolchain.execute(
        ["split", "--make-full-disasm-for-code", str(root / manifest.splat)],
        capture_output=not args.verbose,
        text=not args.verbose,
    )
    if not args.verbose:
        if result.returncode:
            if result.stdout:
                print(result.stdout, end="", file=sys.stderr)
            if result.stderr:
                print(result.stderr, end="", file=sys.stderr)
        else:
            _project_regenerated_stubs(root, manifest)
            print(f"{manifest.id.value}: splat OK")
    elif result.returncode == 0:
        _project_regenerated_stubs(root, manifest)
    return result.returncode


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="splat")
    add_root_argument(parser)
    parser.add_argument("target", help="target id, for example exe/logo")
    parser.add_argument("--example", action="store_true")
    parser.add_argument("--verbose", action="store_true", help="show full Splat output")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    arguments = sys.argv[1:] if argv is None else argv
    if "--example" in arguments:
        print("bin/splat exe/logo")
        return 0
    return run_main(build_parser, arguments)


if __name__ == "__main__":
    raise SystemExit(main())
