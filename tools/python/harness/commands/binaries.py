"""Materialize reviewed executable payloads and EMI target images."""

from __future__ import annotations

import argparse
from pathlib import Path

from harness.common.cli import add_root_argument, run_main
from harness.common.files import atomic_write, read_file
from harness.domain.manifests import load_target_manifests
from harness.domain.psx import is_psx_exe, payload_for, validate_psx_header
from harness.emi.catalog import load_catalog
from harness.emi.catalog_bootstrap import materialize_reviewed_targets


def materialize_executables(root: Path, *, force: bool = False) -> list[Path]:
    root = root.resolve()
    extracted = root / "out" / "extracted"
    images: list[Path] = []
    pending: list[tuple[str, bytes, bytes | None]] = []
    for manifest in load_target_manifests(root).values():
        if manifest.kind != "executable":
            continue
        source = (extracted / manifest.disc_id).resolve()
        if not source.is_relative_to(extracted) or not source.is_file():
            raise FileNotFoundError(
                f"missing extracted executable for {manifest.id}: {source}"
            )
        original = read_file(root, source.relative_to(root).as_posix())
        if original is None or not is_psx_exe(original):
            raise ValueError(f"missing PS-X EXE header: {source}")
        validate_psx_header(
            original, manifest.load_address, binary_name=manifest.disc_id
        )
        bounds = payload_for(
            original, manifest.load_address, binary_name=manifest.disc_id
        )
        payload = original[bounds.binary_offset :]
        name = str(manifest.binary)
        current = read_file(root, name, missing_ok=True)
        if current != payload:
            if current is not None and current != original and not force:
                raise ValueError(
                    f"normalized binary differs from {manifest.disc_id}: {name}; "
                    "inspect it before explicitly forcing replacement"
                )
            pending.append((name, payload, current))
        images.append(root / name)
    for name, payload, current in pending:
        quarantine = atomic_write(root, name, payload, expected=current)
        if quarantine is not None:
            print(f"binaries: retained previous {name} at {quarantine}")
    return images


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    images = materialize_executables(root, force=args.force)
    images.extend(materialize_reviewed_targets(root=root, catalog=load_catalog(root)))
    print(f"binaries: {len(images)} images ready")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="binaries", description="Restore reviewed images from extracted media."
    )
    add_root_argument(parser)
    parser.add_argument(
        "--force", action="store_true", help="replace differing executable images"
    )
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
