from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path
from typing import Any

from ..binaries import (
    normalize_executable,
    verify_splat_hash,
)
from ..targets import (
    materialize_promoted_emi_targets,
    promote_entry,
    write_catalog,
)
from ..io import read_json, repo_layout
from ._common import run_main
from .disc import configure_unpack_parser
from .setup import (
    add_setup_option_flags,
    run_flat_command as run_setup,
    setup_task_names,
)


def _target_manifests(args: argparse.Namespace):
    from ..targets import load_target_manifests

    return load_target_manifests(_root(args))


def _root(args: argparse.Namespace) -> Path:
    return args.root.resolve()


def _catalog_path(args: argparse.Namespace) -> Path:
    return _root(args) / "out" / "catalog" / "emi.json"


def run_discover(args: argparse.Namespace) -> int:
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


def run_targets(args: argparse.Namespace) -> int:
    from ..targets import load_target_manifests, normalize_target_id

    root = _root(args)
    manifests = load_target_manifests(root)
    target_id_str = getattr(args, "target", None)
    if target_id_str is None:
        if args.json:
            print(
                json.dumps(
                    [
                        {
                            "id": target_id,
                            "disc_id": manifest.disc_id,
                            "kind": manifest.kind,
                            "profile": manifest.profile,
                        }
                        for target_id, manifest in sorted(manifests.items())
                    ],
                    indent=2,
                    sort_keys=True,
                )
            )
            return 0
        for target_id, manifest in sorted(manifests.items()):
            print(f"{target_id}\t{manifest.disc_id}\t{manifest.kind}\t{manifest.profile}")
        return 0

    target_id = normalize_target_id(target_id_str).value
    manifest = manifests.get(target_id)
    if manifest is None:
        raise ValueError(f"unknown target: {args.target}")
    payload = {
        "id": manifest.id.value,
        "disc_id": manifest.disc_id,
        "kind": manifest.kind,
        "source_dir": manifest.source_dir,
        "binary": manifest.binary,
        "splat": manifest.splat,
        "load_address": manifest.load_address,
        "profile": manifest.profile,
        "psyq_headers": manifest.psyq_headers,
        "psyq_libraries": {
            name: {
                "members": list(members),
                "confidence": manifest.library_confidence.get(name) or None,
                "evidence": list(manifest.library_evidence.get(name, ())),
            }
            for name, members in sorted(manifest.libraries.items())
        },
    }
    print(
        json.dumps(payload, indent=2, sort_keys=True)
        if args.json
        else "\n".join(f"{key}: {value}" for key, value in payload.items())
    )
    return 0


def run_reverse(args: argparse.Namespace) -> int:
    from .. import reverse
    from ..domain import load_target_manifests, normalize_target_id, parse_function_id

    root = _root(args)
    target_str = getattr(args, "target", None)

    if target_str is None:
        print("Usage: harness reverse TARGET[@ADDRESS]")
        manifests = load_target_manifests(root)
        if manifests:
            print("Active targets:")
            for target_id, manifest in sorted(manifests.items()):
                print(
                    f"  {target_id}\t{manifest.disc_id}\t{manifest.kind}\t{manifest.profile}"
                )
        else:
            print("No active targets found.")
        return 0

    manifests = load_target_manifests(root)
    if "@" in target_str:
        try:
            function_id = parse_function_id(target_str)
        except ValueError as exc:
            raise ValueError(f"invalid reverse target {target_str!r}: {exc}") from exc
        target_id = function_id.target.value
        address = function_id.address
    else:
        target_id = normalize_target_id(target_str).value
        address = None
    if target_id not in manifests:
        raise ValueError(f"unknown target: {target_id}")

    plan_kwargs: dict[str, Any] = {"strategy": args.strategy}
    if args.goal is not None:
        plan_kwargs["goal"] = args.goal
    if args.functions is not None:
        plan_kwargs["budget_functions"] = args.functions
    if args.time is not None:
        plan_kwargs["budget_time_seconds"] = args.time * 60
    if args.depth is not None:
        plan_kwargs["budget_depth"] = args.depth

    mission = reverse.plan_mission(root, target_id, address, **plan_kwargs)
    preview = reverse.preview_mission(root, mission)

    print(f"Mission ID: {mission.mission_id}")
    print(f"Target: {preview['target']}")
    if preview["address"] is not None:
        print(f"Address: {preview['address']:#010x}")
    print(f"Inferred goal: {preview['inferred_goal']}")
    print(f"Strategy: {preview['strategy']}")
    budget = preview["budget"]
    print(
        f"Budget: functions={budget['functions']}, "
        f"time={budget['time_seconds'] // 60}m, depth={budget['depth']}"
    )
    print(f"Status: {mission.status}")

    if args.run:
        from ..agents import run_mission
        bundle = run_mission(root, mission.to_dict())
        import json
        print(json.dumps(bundle, indent=2, sort_keys=True))

    return 0


def run_diff(args: argparse.Namespace) -> int:
    from ..targets import load_target_manifests, parse_function_id
    from ..match.asm_diff import AsmDiffRequest, run_asm_diff_one
    from ..match.asm_differ import write_bundle
    from ._asm_diff_output import format_asm_diff_llm, format_asm_diff_summary

    if args.llm and (args.json or args.show_diff):
        raise ValueError("--llm cannot be combined with --json or --show-diff")

    root = _root(args)
    source = Path(args.source)
    request_kwargs: dict[str, Any] = {"output_root": root / "out" / "matching"}
    if not source.is_file():
        function = parse_function_id(str(args.source))
        manifest = load_target_manifests(root).get(function.target.value)
        if manifest is None:
            raise ValueError(f"unknown target: {function.target.value}")
        source = root / manifest.source_dir / f"func_{function.address:08x}.c"
        request_kwargs.update(
            binary_path=root / manifest.binary,
            load_address=manifest.load_address,
            address=function.address,
        )
    payload = run_asm_diff_one(AsmDiffRequest(source_path=source, **request_kwargs))
    write_bundle(root, payload, html_output=args.html)
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    elif args.llm:
        print(format_asm_diff_llm(payload, root=root))
    else:
        print(format_asm_diff_summary(payload, root=root))
        if args.show_diff:
            print(Path(payload["outputs"]["diff"]).read_text(encoding="utf-8"))
    return 0 if payload["exact_match"] else 1


def run_normalize(args: argparse.Namespace) -> int:
    root = _root(args)
    for name, source in (("slus_004_22", args.slus), ("logo", args.logo)):
        image = root / "out" / "binaries" / "exe" / f"{name}.bin"
        metadata = normalize_executable(source, image)
        verify_splat_hash(
            root / "config" / "splat" / "exe" / f"{name}.yaml", image
        )
        print(f"normalized {name}: {metadata['image']}")
    catalog = write_catalog(root / "out" / "extracted" / "BIN", _catalog_path(args))
    for image in materialize_promoted_emi_targets(root=root, catalog=catalog):
        print(f"normalized EMI: {image}")
    return 0


def run_doctor(args: argparse.Namespace) -> int:
    root = _root(args)
    from ..targets import load_profiles, load_target_manifests

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
        manifests = load_target_manifests(root)
        profiles = load_profiles(root)
        for target_id, manifest in manifests.items():
            if manifest.profile not in profiles:
                invalid.append(f"unknown profile {manifest.profile} for {target_id}")
            if not (root / manifest.source_dir).is_dir():
                invalid.append(f"missing source directory {manifest.source_dir}")
        manifests_by_splat = {
            manifest.splat: manifest for manifest in manifests.values()
        }
        for config in sorted((root / "config" / "splat").rglob("*.yaml")):
            text = config.read_text(encoding="utf-8")
            relative_config = config.relative_to(root).as_posix()
            manifest = manifests_by_splat.get(relative_config)
            if (
                manifest is None or manifest.status == "active"
            ) and re.search(r"^\s+-\s*\[\s*0x0\s*,\s*bin\s*\]", text, re.M):
                invalid.append(f"tracked bootstrap layout not allowed: {config.relative_to(root)}")
                continue
            target_match = re.search(r"^  target_path: (.+)$", text, re.M)
            hash_match = re.search(r"^sha1: ([0-9a-f]{40})$", text, re.M)
            if target_match is None or hash_match is None:
                invalid.append(f"incomplete {config.relative_to(root)}")
                continue
            if manifest is not None and manifest.status == "quarantined":
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
        else "configuration ready; run harness discover after extraction"
    )
    return 0


def run_assets_list(args: argparse.Namespace) -> int:
    catalog = read_json(_catalog_path(args))
    for kind, count in catalog["payload_kind_counts"].items():
        print(f"{kind}: {count}")
    return 0


def _str_output_dir(args: argparse.Namespace) -> Path:
    source = args.source if args.source.is_absolute() else _root(args) / args.source
    if args.output_dir is None:
        return _root(args) / "out" / "assets" / "str" / source.stem
    return (
        args.output_dir
        if args.output_dir.is_absolute()
        else _root(args) / args.output_dir
    )


def run_assets_str_validate(args: argparse.Namespace) -> int:
    import shutil

    from ..assets.str_media import validate_str

    source = args.source if args.source.is_absolute() else _root(args) / args.source
    result = validate_str(
        source,
        _str_output_dir(args),
        expected_fps=args.expected_fps,
        ffprobe=shutil.which("ffprobe"),
    )
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        print(
            f"STR {result['status']} sectors={result['sector_count']} "
            f"frames={result['frame_count']} audio={len(result['audio_streams'])} "
            f"manifest={result['manifest']}"
        )
    return 1 if result["status"] == "fail" else 0


def run_assets_str_convert(args: argparse.Namespace) -> int:
    from ..assets.str_media import convert_str

    source = args.source if args.source.is_absolute() else _root(args) / args.source
    output = args.output
    if output is not None and not output.is_absolute():
        output = _root(args) / output
    result = convert_str(
        source,
        _str_output_dir(args),
        fps=args.fps,
        output=output,
    )
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        print(f"STR converted output={result['output']} manifest={result['manifest']}")
    return 1 if result.get("status") == "fail" else 0


def run_psyq_import_command(args: argparse.Namespace) -> int:
    from .toolchain import run_psyq_import

    return run_psyq_import(args)


def build_parser() -> argparse.ArgumentParser:
    layout = repo_layout()
    parser = argparse.ArgumentParser(prog="harness")
    parser.add_argument("--root", type=Path, default=layout.root)
    sub = parser.add_subparsers(dest="command", required=True)

    setup = sub.add_parser("setup", help="set up the workspace (plan/workspace/task)")
    setup.add_argument("--plan", action="store_true")
    setup.add_argument("--open", dest="open_setup", action="store_true")
    setup.add_argument("--task", dest="task_name", choices=setup_task_names())
    add_setup_option_flags(
        setup,
        include_force=True,
        include_psyq_inputs=True,
        include_skip_flags=True,
    )
    setup.set_defaults(handler=run_setup)

    discover = sub.add_parser("discover", help="scan disc inputs for binaries and entries")
    discover.add_argument(
        "--emi-root", type=Path, default=layout.out_dir / "extracted" / "BIN"
    )
    discover.set_defaults(handler=run_discover)

    promote = sub.add_parser("promote", help="promote an EMI archive entry to a confirmed module")
    promote.add_argument("target")
    promote.add_argument("--confirm-code", action="store_true")
    promote.set_defaults(handler=run_promote)

    targets = sub.add_parser("targets", help="list all targets or show one target")
    targets.add_argument("target", nargs="?")
    targets.add_argument("--json", action="store_true")
    targets.set_defaults(handler=run_targets)

    reverse = sub.add_parser("reverse", help="reverse-engineer a target or function")
    reverse.add_argument("target", nargs="?")
    reverse.add_argument(
        "--goal", choices=["understand", "lift", "improve", "match", "complete"]
    )
    reverse.add_argument(
        "--strategy",
        choices=["balanced", "hot", "leaf", "root", "shallow", "easy"],
        default="balanced",
    )
    reverse.add_argument("--functions", type=int)
    reverse.add_argument("--time", type=int, help="duration in minutes")
    reverse.add_argument("--depth", type=int)
    reverse.add_argument("--run", action="store_true", help="start the mission")
    reverse.set_defaults(handler=run_reverse)

    diff = sub.add_parser("diff", help="compare a lifted source against the original bytes")
    diff.add_argument("source", type=Path)
    diff.add_argument("--json", action="store_true")
    diff.add_argument("--show", "--show-diff", dest="show_diff", action="store_true")
    diff.add_argument(
        "--llm",
        action="store_true",
        help="print a bounded first diff hunk and the full artifact path",
    )
    diff.add_argument("--html", action="store_true")
    diff.add_argument("--watch", action="store_true")
    diff.set_defaults(handler=run_diff)

    doctor = sub.add_parser("doctor", help="check workspace/toolchain health")
    doctor.add_argument("--strict", action="store_true")
    doctor.set_defaults(handler=run_doctor)

    assets = sub.add_parser("assets", help="list/validate/convert assets (list/str)")
    assets_sub = assets.add_subparsers(dest="assets_command", required=True)
    assets_list = assets_sub.add_parser("list")
    assets_list.set_defaults(handler=run_assets_list)
    assets_str = assets_sub.add_parser("str")
    assets_str_sub = assets_str.add_subparsers(dest="assets_str_command", required=True)
    assets_str_validate = assets_str_sub.add_parser("validate")
    assets_str_validate.add_argument("source", type=Path)
    assets_str_validate.add_argument("--expected-fps", type=float)
    assets_str_validate.add_argument("--output-dir", type=Path)
    assets_str_validate.add_argument("--json", action="store_true")
    assets_str_validate.set_defaults(handler=run_assets_str_validate)
    assets_str_convert = assets_str_sub.add_parser("convert")
    assets_str_convert.add_argument("source", type=Path)
    assets_str_convert.add_argument("--fps", type=float, required=True)
    assets_str_convert.add_argument("--output", type=Path)
    assets_str_convert.add_argument("--output-dir", type=Path)
    assets_str_convert.add_argument("--json", action="store_true")
    assets_str_convert.set_defaults(handler=run_assets_str_convert)

    psyq = sub.add_parser("psyq", help="import the PsyQ SDK (import)")
    psyq_sub = psyq.add_subparsers(dest="psyq_command", required=True)
    psyq_import = psyq_sub.add_parser("import")
    psyq_import.add_argument("--version", default="4.7")
    psyq_import.add_argument("--archive", type=Path)
    psyq_import.add_argument("--archive-url")
    psyq_import.add_argument("--dest", type=Path)
    psyq_import.add_argument("--private-root", type=Path)
    psyq_import.add_argument("--force", action="store_true")
    psyq_import.set_defaults(handler=run_psyq_import_command)

    emi = sub.add_parser("emi", help="unpack and inspect EMI archives (unpack)")
    emi_sub = emi.add_subparsers(dest="emi_command", required=True)
    configure_unpack_parser(emi_sub.add_parser("unpack"))

    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
