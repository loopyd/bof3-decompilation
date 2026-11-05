"""Run Splat for exactly one manifest-owned target."""

from __future__ import annotations

import re

import json
import argparse
import sys
from pathlib import Path

from harness.common.cli import add_example_argument, add_root_argument, run_main

from ..domain.layout import parse_splat_layout
from ..domain.registry import lookup_target_manifest
from ..domain.sources import LiftMetadataError, lift_metadata
from ..io import repo_layout
from ..toolchain.splat import SplatToolchain


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


def _sdk_text(object_path: Path, root: Path) -> bytes | None:
    """Return an SDK object's .text bytes, or None when it holds no .text."""
    import subprocess

    executable = repo_layout(root).root / "bin" / "objcopy"
    if not executable.is_file():
        return None
    result = subprocess.run(
        [str(executable), "-O", "binary", "--only-section=.text", str(object_path), "/dev/stdout"],
        capture_output=True,
    )
    if result.returncode != 0 or not result.stdout:
        return None
    return result.stdout


def _sdk_slot_report(root: Path, manifest) -> list[dict]:
    """Check every SDK-owned asm boundary against the SDK object that owns it.

    A slot whose shipped bytes appear in a declared SDK object builds from that
    object; one that does not is an ``sdk_variant`` - the shipped library was a
    different build - which is recorded rather than treated as a failure.
    """

    layout_text = (root / manifest.splat).read_text(encoding="utf-8")
    sdk_map: dict[int, str] = {}
    for entry in re.findall(r'^\s*- (config/sdk/\S+\.txt)\s*$', layout_text, re.M):
        for line in (root / entry).read_text(encoding="utf-8").splitlines():
            match = re.match(r"\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;", line)
            if match:
                sdk_map[int(match.group(2), 16)] = match.group(1)
    if not sdk_map:
        return []
    # Claimed members are a reviewed sample, not every object the slot may come
    # from, so the whole staged SDK is searched before calling a slot a variant.
    claimed = re.findall(
        r'"((?:[^"]*/)?psyq/[^"]+\.(?:o|obj))"', (root / "config/targets" / manifest.id.value / "target.toml").read_text(encoding="utf-8")
    )
    members = list(claimed)
    for library in sorted((root / "toolchains" / "psyq").glob("*/")):
        for directory in sorted(library.glob("lib*")):
            for obj in sorted(directory.glob("*.o")):
                members.append(obj.relative_to(root / "toolchains").as_posix())
    payload = (root / manifest.binary).read_bytes()
    layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    asm_names = {
        path.stem: path
        for path in (root / "out" / "splat" / manifest.id.value / "asm").glob("*.s")
    }
    texts = {}
    rows = []
    for boundary in layout.boundaries:
        if boundary.kind != "asm" or boundary.virtual_start not in sdk_map:
            continue
        start = boundary.virtual_start - manifest.load_address
        size = boundary.file_size
        body = payload[start : start + size]
        if not body or not size:
            continue
        owner = None
        for member in members:
            obj = root / "toolchains" / member
            if not obj.is_file():
                continue
            if obj not in texts:
                texts[obj] = _sdk_text(obj, root) or b""
            index = texts[obj].find(body)
            if index >= 0:
                owner = (member, index)
                break
        rows.append(
            {
                "function": f"0x{boundary.virtual_start:08X}",
                "name": sdk_map[boundary.virtual_start],
                "symbol": (asm_names.get(boundary.name or "", Path()).stem or None),
                "bytes": size,
                "status": "exact_sdk" if owner else "sdk_variant",
                "object": owner[0] if owner else None,
                "object_offset": f"0x{owner[1]:X}" if owner else None,
            }
        )
    destination = root / "out" / "psyq" / f"slots-{manifest.id.value.replace('/', '-')}.json"
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(json.dumps(rows, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    for row in rows:
        where = f"{row['object']}@{row['object_offset']}" if row["object"] else "no matching sdk object"
        print(f"sdk-slot {row['status']} {row['function']} {row['name']} ({row['bytes']} B) {where}")
    return rows


def _summary(root: Path, manifest) -> str:
    """Report what the split produced, not just that it exited zero.

    A payload declared as one opaque ``bin`` segment splits successfully and still
    produces no assembly. Saying "OK" alone sends the caller to re-run a split that
    cannot ever satisfy it.
    """

    asm = root / "out" / "splat" / manifest.id.value / "asm"
    files = list(asm.glob("*.s")) if asm.is_dir() else []
    if files:
        return f"splat OK ({len(files)} asm files)"
    return "splat OK (no declared asm function boundaries; nothing to emit)"


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    manifest = lookup_target_manifest(root, args.target)
    if manifest is None:
        raise ValueError(f"unknown target: {args.target}")
    toolchain = SplatToolchain(repo_layout(root))
    if not toolchain.executable.is_file():
        raise FileNotFoundError(
            f"missing Splat executable: {toolchain.executable}; run just setup"
        )
    _assert_pre_run_safety(root, manifest)
    output = root / "out" / "splat" / manifest.id.value
    output.mkdir(parents=True, exist_ok=True)
    result = toolchain.execute(
        [
            "split",
            "--make-full-disasm-for-code",
            str(root / manifest.splat),
            str(root / "config" / "splat.yaml"),
        ],
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
            print(f"{manifest.id.value}: {_summary(root, manifest)}")
            _sdk_slot_report(root, manifest)
    elif result.returncode == 0:
        _project_regenerated_stubs(root, manifest)
        _sdk_slot_report(root, manifest)
    return result.returncode


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness source splat")
    add_root_argument(parser)
    parser.add_argument("target", help="target id, for example exe/logo")
    add_example_argument(parser, "bin/harness source splat exe/logo")
    parser.add_argument("--verbose", action="store_true", help="show full Splat output")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
