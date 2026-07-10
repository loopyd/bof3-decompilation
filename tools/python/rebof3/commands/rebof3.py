from __future__ import annotations

import argparse
import hashlib
import re
from collections import Counter
from pathlib import Path

from ..binaries import (
    normalize_executable,
    parse_number,
    promote_entry,
    record_lift,
    set_splat_expected_hash,
    write_catalog,
)
from ..jsonio import read_json
from ..paths import repo_layout
from ._common import run_main


def _root(args: argparse.Namespace) -> Path:
    return args.root.resolve()


def _catalog_path(args: argparse.Namespace) -> Path:
    return _root(args) / "out" / "catalog" / "emi.json"


def run_scan(args: argparse.Namespace) -> int:
    catalog = write_catalog(args.emi_root, _catalog_path(args))
    print(f"catalog: {_catalog_path(args)}")
    print(f"archives: {catalog['archive_count']}; entries: {catalog['entry_count']}")
    return 0


def run_promote(args: argparse.Namespace) -> int:
    config, source = promote_entry(
        catalog_path=_catalog_path(args),
        identifier=args.target,
        root=_root(args),
        confirm_code=args.confirm_code,
    )
    print(f"promoted: {config.relative_to(_root(args))}")
    print(f"source: {source.relative_to(_root(args))}")
    return 0


def run_candidates(args: argparse.Namespace) -> int:
    catalog = read_json(_catalog_path(args))
    rows = [
        entry
        for entry in catalog["entries"]
        if entry["code_status"] in {"candidate", "unknown"}
    ]
    if args.family:
        rows = [
            entry for entry in rows if entry["family"].lower() == args.family.lower()
        ]
    for entry in sorted(
        rows, key=lambda item: (-item["evidence"]["instruction_density"], item["id"])
    ):
        print(
            f"{entry['id']} {entry['load_address_hex']} {entry['code_status']} density={entry['evidence']['instruction_density']}"
        )
    return 0


def run_status(args: argparse.Namespace) -> int:
    catalog = read_json(_catalog_path(args))
    if args.target:
        from ..binaries import resolve_entry

        entry = resolve_entry(catalog, args.target)
        print(
            f"{entry['id']}: {entry['payload_kind']}, {entry['code_status']}, {entry['load_address_hex']}, {entry['size']} bytes"
        )
    else:
        print(
            f"archives: {catalog['archive_count']}; entries: {catalog['entry_count']}"
        )
        print(
            "code status: "
            + ", ".join(
                f"{key}={value}"
                for key, value in sorted(
                    Counter(
                        entry["code_status"] for entry in catalog["entries"]
                    ).items()
                )
            )
        )
    return 0


def run_next(args: argparse.Namespace) -> int:
    catalog = read_json(_catalog_path(args))
    rows = [
        entry for entry in catalog["entries"] if entry["code_status"] == "confirmed"
    ]
    if args.target:
        rows = [entry for entry in rows if entry["id"] == args.target]
    if not rows:
        raise ValueError(
            "no confirmed target; run rebof3 promote <archive#slot> --confirm-code first"
        )
    entry = sorted(rows, key=lambda item: item["id"])[0]
    print(f"rebof3 lift {entry['id']}@{entry['load_address_hex']}")
    return 0


def run_lift(args: argparse.Namespace) -> int:
    target, separator, raw_address = args.target.rpartition("@")
    if not separator:
        raise ValueError("lift target must be TARGET@ADDRESS")
    source = record_lift(
        root=_root(args),
        catalog_path=_catalog_path(args),
        target=target,
        address=parse_number(raw_address),
    )
    print(source.relative_to(_root(args)))
    return 0


def run_normalize(args: argparse.Namespace) -> int:
    root = _root(args)
    for name, source in (("slus_004_22", args.slus), ("logo", args.logo)):
        image = root / "out" / "binaries" / "exe" / f"{name}.bin"
        metadata = normalize_executable(source, image)
        set_splat_expected_hash(
            root / "config" / "splat" / "exe" / f"{name}.yaml", image
        )
        print(f"normalized {name}: {metadata['image']}")
    return 0


def run_diff(args: argparse.Namespace) -> int:
    from . import asm_diff

    return asm_diff.main([str(args.source)])


def run_ghidra_sync(args: argparse.Namespace) -> int:
    catalog = read_json(_catalog_path(args))
    imports = [
        {
            "target": entry["id"],
            "binary": entry["payload_path"],
            "load_address": entry["load_address"],
        }
        for entry in catalog["entries"]
        if entry["code_status"] == "confirmed"
    ]
    output = _root(args) / "out" / "ghidra" / "imports.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    from ..jsonio import write_json

    write_json(output, {"schema": "rebof3.ghidra-imports/v1", "imports": imports})
    print(f"Ghidra import manifest: {output}")
    return 0


def run_assets_list(args: argparse.Namespace) -> int:
    catalog = read_json(_catalog_path(args))
    for kind, count in catalog["payload_kind_counts"].items():
        print(f"{kind}: {count}")
    return 0


def run_disk_verify(args: argparse.Namespace) -> int:
    from . import disk

    return disk.main(["disk-verify"])


def run_disk_rebuild(args: argparse.Namespace) -> int:
    from . import disk

    return disk.main(["disk-rebuild"])


def run_doctor(args: argparse.Namespace) -> int:
    root = _root(args)
    missing = [
        path
        for path in (
            root / "config" / "splat" / "exe" / "slus_004_22.yaml",
            root / "config" / "splat" / "exe" / "logo.yaml",
            root / "config" / "symbols" / "shared.txt",
        )
        if not path.is_file()
    ]
    if missing:
        raise ValueError(
            "missing tracked configuration: "
            + ", ".join(str(path.relative_to(root)) for path in missing)
        )
    if args.strict:
        invalid: list[str] = []
        for config in sorted((root / "config" / "splat").rglob("*.yaml")):
            text = config.read_text(encoding="utf-8")
            target_match = re.search(r"^  target_path: (.+)$", text, re.M)
            hash_match = re.search(r"^sha1: ([0-9a-f]{40})$", text, re.M)
            if target_match is None or hash_match is None:
                invalid.append(f"incomplete {config.relative_to(root)}")
                continue
            target = root / target_match.group(1)
            if not target.is_file():
                invalid.append(f"missing {target_match.group(1)}")
            elif hashlib.sha1(target.read_bytes()).hexdigest() != hash_match.group(1):
                invalid.append(f"hash mismatch {config.relative_to(root)}")
        if invalid:
            raise ValueError("strict doctor failed: " + "; ".join(invalid))
    print(
        "READY"
        if _catalog_path(args).is_file()
        else "configuration ready; run rebof3 scan after extraction"
    )
    return 0


def build_parser() -> argparse.ArgumentParser:
    layout = repo_layout()
    parser = argparse.ArgumentParser(prog="rebof3")
    parser.add_argument("--root", type=Path, default=layout.root)
    sub = parser.add_subparsers(dest="command", required=True)
    scan = sub.add_parser("scan")
    scan.add_argument(
        "--emi-root", type=Path, default=layout.out_dir / "extracted" / "BIN"
    )
    scan.set_defaults(handler=run_scan)
    promote = sub.add_parser("promote")
    promote.add_argument("target")
    promote.add_argument("--confirm-code", action="store_true")
    promote.set_defaults(handler=run_promote)
    candidates = sub.add_parser("candidates")
    candidates.add_argument("family", nargs="?")
    candidates.set_defaults(handler=run_candidates)
    status = sub.add_parser("status")
    status.add_argument("target", nargs="?")
    status.set_defaults(handler=run_status)
    nxt = sub.add_parser("next")
    nxt.add_argument("target", nargs="?")
    nxt.set_defaults(handler=run_next)
    lift = sub.add_parser("lift")
    lift.add_argument("target")
    lift.set_defaults(handler=run_lift)
    normalize = sub.add_parser("normalize")
    normalize.add_argument(
        "--slus", type=Path, default=layout.out_dir / "extracted" / "SLUS_004.22"
    )
    normalize.add_argument(
        "--logo", type=Path, default=layout.out_dir / "extracted" / "LOGO" / "LOGO.EXE"
    )
    normalize.set_defaults(handler=run_normalize)
    doctor = sub.add_parser("doctor")
    doctor.add_argument("--strict", action="store_true")
    doctor.set_defaults(handler=run_doctor)
    diff = sub.add_parser("diff")
    diff.add_argument("source", type=Path)
    diff.set_defaults(handler=run_diff)
    permute = sub.add_parser("permute")
    permute.add_argument("source", type=Path)
    permute.set_defaults(
        handler=lambda args: (_ for _ in ()).throw(
            ValueError(
                "permutation is available after a function has a buildable C target"
            )
        )
    )
    ghidra = sub.add_parser("ghidra")
    ghidra_sub = ghidra.add_subparsers(dest="ghidra_command", required=True)
    ghidra_sync = ghidra_sub.add_parser("sync")
    ghidra_sync.set_defaults(handler=run_ghidra_sync)
    ghidra_ui = ghidra_sub.add_parser("ui")
    ghidra_ui.set_defaults(
        handler=lambda args: (_ for _ in ()).throw(
            ValueError(
                "set GHIDRA_HOME and use bin/ghidra-ui until the loader is installed"
            )
        )
    )
    assets = sub.add_parser("assets")
    assets_sub = assets.add_subparsers(dest="assets_command", required=True)
    assets_list = assets_sub.add_parser("list")
    assets_list.set_defaults(handler=run_assets_list)
    for name in ("extract", "preview"):
        command = assets_sub.add_parser(name)
        command.set_defaults(
            handler=lambda args, action=name: (_ for _ in ()).throw(
                ValueError(f"use bin/emi-{action} for this asset operation")
            )
        )
    disk = sub.add_parser("disk")
    disk_sub = disk.add_subparsers(dest="disk_command", required=True)
    disk_verify = disk_sub.add_parser("verify")
    disk_verify.set_defaults(handler=run_disk_verify)
    disk_rebuild = disk_sub.add_parser("rebuild")
    disk_rebuild.set_defaults(handler=run_disk_rebuild)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
