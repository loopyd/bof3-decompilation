from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import tempfile
import time
from collections import Counter
from pathlib import Path
from typing import Any

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


def _target_manifests(args: argparse.Namespace):
    from ..domain import load_target_manifests

    return load_target_manifests(_root(args))


def run_target_list(args: argparse.Namespace) -> int:
    manifests = _target_manifests(args)
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


def run_target_show(args: argparse.Namespace) -> int:
    from ..domain import normalize_target_id

    target_id = normalize_target_id(args.target)
    manifest = _target_manifests(args).get(target_id.value)
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


def run_index_build(args: argparse.Namespace) -> int:
    from ..evidence import build_index
    from ..registry import generate_build_manifest

    generate_build_manifest(_root(args))
    summary = build_index(_root(args))
    print(
        json.dumps(summary, sort_keys=True)
        if args.json
        else f"index: {summary['database']} targets={summary['targets']} functions={summary['functions']} edges={summary['edges']}"
    )
    return 0


def _index_connection(args: argparse.Namespace):
    from ..evidence import connect_index

    path = _root(args) / "out" / "index" / "harness.sqlite"
    if not path.is_file():
        raise FileNotFoundError("evidence index missing; run `harness index build`")
    return connect_index(path)


def run_index_find(args: argparse.Namespace) -> int:
    from ..evidence import find_records

    connection = _index_connection(args)
    try:
        term = args.value or args.term
        if not term:
            raise ValueError("find requires a term or --value")
        rows = find_records(connection, term)
    finally:
        connection.close()
    if args.json:
        print(json.dumps(rows, indent=2, sort_keys=True))
    else:
        for row in rows:
            print(f"{row['table']}: {row.get('id', '')}")
    return 0


def run_index_show(args: argparse.Namespace) -> int:
    connection = _index_connection(args)
    try:
        rows = []
        for table in ("targets", "functions", "symbols", "types", "declarations"):
            row = connection.execute(
                f"SELECT * FROM {table} WHERE id = ?", (args.identifier,)
            ).fetchone()
            if row is not None:
                item = {"table": table, **dict(row)}
                for key, value in tuple(item.items()):
                    if isinstance(value, bytes):
                        item[key] = value.hex()
                rows.append(item)
    finally:
        connection.close()
    if not rows:
        raise ValueError(f"unknown evidence ID: {args.identifier}")
    print(
        json.dumps(rows[0], indent=2, sort_keys=True)
        if args.json
        else "\n".join(f"{key}: {value}" for key, value in rows[0].items())
    )
    return 0


def run_index_related(args: argparse.Namespace) -> int:
    connection = _index_connection(args)
    try:
        rows = connection.execute(
            "SELECT relation, target_id FROM edges WHERE source_id = ? UNION ALL SELECT relation, source_id FROM edges WHERE target_id = ? ORDER BY relation, target_id",
            (args.identifier, args.identifier),
        ).fetchall()
    finally:
        connection.close()
    for row in rows:
        print(f"{row['relation']} {row['target_id']}")
    return 0


def run_graph(args: argparse.Namespace) -> int:
    connection = _index_connection(args)
    try:
        frontier = {args.identifier}
        if args.identifier.startswith("type:"):
            name = args.identifier.split(":", 1)[1]
            frontier = {
                str(row["id"])
                for row in connection.execute(
                    "SELECT id FROM types WHERE name LIKE ?", (f"%{name}%",)
                )
            }
            if not frontier:
                raise ValueError(f"unknown graph type: {name}")
        seen = set(frontier)
        rows: list[dict[str, str | int]] = []
        for depth in range(1, args.depth + 1):
            next_frontier: set[str] = set()
            for source_id in sorted(frontier):
                for row in connection.execute(
                    "SELECT relation, target_id FROM edges WHERE source_id = ? "
                    "UNION ALL SELECT relation, source_id FROM edges WHERE target_id = ?",
                    (source_id, source_id),
                ):
                    target_id = str(row["target_id"])
                    rows.append(
                        {
                            "depth": depth,
                            "relation": row["relation"],
                            "target_id": target_id,
                        }
                    )
                    if target_id not in seen:
                        seen.add(target_id)
                        next_frontier.add(target_id)
            frontier = next_frontier
    finally:
        connection.close()
    if args.json:
        print(json.dumps(rows, indent=2, sort_keys=True))
    else:
        for row in rows:
            print(f"{row['depth']}: {row['relation']} {row['target_id']}")
    return 0


def run_path(args: argparse.Namespace) -> int:
    connection = _index_connection(args)
    try:
        queue = [args.left]
        parent: dict[str, str | None] = {args.left: None}
        while queue and args.right not in parent:
            source_id = queue.pop(0)
            neighbors = connection.execute(
                "SELECT target_id FROM edges WHERE source_id = ? "
                "UNION SELECT source_id FROM edges WHERE target_id = ? ORDER BY target_id",
                (source_id, source_id),
            )
            for row in neighbors:
                target_id = str(row["target_id"])
                if target_id not in parent:
                    parent[target_id] = source_id
                    queue.append(target_id)
        if args.right not in parent:
            raise ValueError(f"no evidence path between {args.left} and {args.right}")
        path = []
        current: str | None = args.right
        while current is not None:
            path.append(current)
            current = parent[current]
        path.reverse()
    finally:
        connection.close()
    print(json.dumps(path) if args.json else "\n".join(path))
    return 0


def run_profile(args: argparse.Namespace) -> int:
    from ..domain import load_profiles

    profiles = load_profiles(_root(args))
    if args.profile_command == "list":
        profile_ids = sorted(profiles)
        if args.json:
            print(json.dumps(profile_ids, indent=2))
        else:
            for profile_id in profile_ids:
                print(profile_id)
        return 0
    if args.profile_command == "show":
        profile = profiles.get(args.profile)
        if profile is None:
            raise ValueError(f"unknown profile: {args.profile}")
        print(
            json.dumps(
                {
                    "id": profile.id,
                    "compiler": profile.compiler,
                    "compiler_flags": profile.compiler_flags,
                    "assembler": profile.assembler,
                    "headers": profile.headers,
                    "objects": profile.objects,
                    "linker": profile.linker,
                    "runner": profile.runner,
                },
                indent=2,
                sort_keys=True,
            )
        )
        return 0
    if args.profile_command == "resolve":
        from ..domain import normalize_target_id

        manifest = _target_manifests(args).get(normalize_target_id(args.target).value)
        if manifest is None:
            raise ValueError(f"unknown target: {args.target}")
        profile_id = os.getenv("HARNESS_PROFILE") or manifest.profile
        profile = profiles.get(profile_id)
        if profile is None:
            raise ValueError(f"unknown target profile: {profile_id}")
        payload = {
            "target": manifest.id.value,
            "profile": profile.id,
            "aspsx_version": os.getenv("HARNESS_ASPSX_VERSION"),
            "compiler": args.compiler
            or os.getenv("HARNESS_COMPILER")
            or profile.compiler,
            "assembler": args.assembler
            or os.getenv("HARNESS_ASSEMBLER")
            or profile.assembler,
            "headers": args.headers
            or os.getenv("HARNESS_HEADERS")
            or manifest.psyq_headers
            or profile.headers,
            "objects": args.objects or os.getenv("HARNESS_OBJECTS") or profile.objects,
            "linker": args.linker or os.getenv("HARNESS_LINKER") or profile.linker,
            "runner": args.runner or os.getenv("HARNESS_RUNNER") or profile.runner,
        }
        print(json.dumps(payload, indent=2, sort_keys=True))
        return 0
    raise ValueError(f"unknown profile command: {args.profile_command}")


def run_context_build(args: argparse.Namespace) -> int:
    from ..workflows.context import build_context
    from ..domain import parse_address

    address = parse_address(args.function)
    metadata = build_context(_root(args), args.target, address, mode=args.mode)
    print(metadata["output"])
    return 0


def run_context_show(args: argparse.Namespace) -> int:
    from ..domain import load_target_manifests, normalize_target_id

    target_id = normalize_target_id(args.target)
    manifest = load_target_manifests(_root(args)).get(target_id.value)
    if manifest is None:
        raise ValueError(f"unknown target: {args.target}")
    metadata_path = (
        _root(args)
        / "out"
        / "context"
        / target_id.value
        / manifest.profile
        / "metadata.json"
    )
    if not metadata_path.is_file():
        raise FileNotFoundError(
            f"context metadata missing: {metadata_path}; run `harness context build`"
        )
    print(metadata_path.read_text(encoding="utf-8"))
    return 0


def run_lift_workflow(args: argparse.Namespace) -> int:
    from ..workflows.lift import lift_function
    from ..domain import parse_address

    target = args.target
    function = args.function
    if function is None and "@" in target:
        target, function = target.rsplit("@", 1)
    if function is None:
        raise ValueError("lift requires TARGET and FUNCTION")
    address = parse_address(function)
    metadata = lift_function(_root(args), target, address, seed=args.seed)
    print(metadata["candidate"])
    return 0


def run_accept(args: argparse.Namespace) -> int:
    from ..workflows.lift import accept_candidate

    from ..domain import parse_function_id

    if "@" in args.function:
        function = parse_function_id(args.function)
        target = function.target.value
        address = function.address
    else:
        target = args.target
        from ..domain import parse_address

        address = parse_address(args.function)
    candidate = Path(args.candidate)
    if not candidate.is_absolute():
        candidate = _root(args) / candidate
    destination = accept_candidate(_root(args), candidate, target, address)
    print(destination.relative_to(_root(args)))
    return 0


def run_permute(args: argparse.Namespace) -> int:
    from ..workflows.permuter import prepare_permuter, run_permuter

    source = args.source
    if "@" not in source and not Path(source).is_absolute():
        source = str(_root(args) / source)
    work_root = args.work_root
    if work_root is not None and not work_root.is_absolute():
        work_root = _root(args) / work_root
    metadata = prepare_permuter(_root(args), source, work_root)
    if args.prepare_only:
        if not args.silent:
            print(f"READY  bundle={metadata['bundle']}")
        return 0
    result = run_permuter(
        _root(args),
        metadata,
        jobs=args.jobs,
        verbose=args.verbose,
        show_errors=args.show_errors,
        show_timings=args.show_timings,
    )
    if not args.silent:
        if args.quiet:
            if result["best"]:
                print(
                    f"BEST  score={result['best']['score']} improvements={result['improvements']}"
                )
            else:
                print("NO IMPROVEMENT")
        elif result["best"]:
            print(
                f"DONE  best={result['best']['score']} improvements={result['improvements']} "
                f"elapsed={result['elapsed_seconds']:.2f}s"
            )
            print(f"candidate: {result['best']['path']}")
        else:
            print("DONE  no improvement")
    return 0 if result["best"] else 1


def run_adopt(args: argparse.Namespace) -> int:
    candidate = Path(args.candidate)
    if not candidate.is_absolute():
        candidate = _root(args) / candidate
    if not candidate.is_file():
        raise FileNotFoundError(f"candidate not found: {candidate}")
    text = candidate.read_text(encoding="utf-8")
    if "PERM" in text or "Pending analysis" in text or "m2c" in text.lower():
        raise ValueError("candidate contains a lift/permuter placeholder")
    if not args.allow_nonmatch:
        summary = candidate.parent / "summary.json"
        if summary.is_file():
            result = json.loads(summary.read_text(encoding="utf-8"))
            if result.get("exact_match") is False:
                raise ValueError(
                    "candidate is not an exact match; pass --allow-nonmatch"
                )
    if not args.apply:
        print("validated; pass --apply to copy the candidate into tracked source")
        return 0
    from ..workflows.lift import accept_candidate

    from ..domain import parse_function_id

    function_id = args.function
    if function_id is None:
        metadata_path = candidate.parent / "metadata.json"
        if not metadata_path.is_file():
            raise ValueError(
                "adopt requires --function unless the candidate has metadata.json"
            )
        metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
        function_id = metadata.get("function")
    if not function_id:
        raise ValueError("candidate metadata is missing its function ID")
    function = parse_function_id(function_id)
    destination = accept_candidate(
        _root(args), candidate, function.target.value, function.address
    )
    print(destination.relative_to(_root(args)))
    return 0


def run_psyq_inventory(args: argparse.Namespace) -> int:
    """Catalog staged PsyQ files while retaining provenance outside SQLite."""

    root = _root(args)
    version = args.version.removeprefix("psyq-").removeprefix("psyq")
    if args.psyq_command == "index":
        from ..psyq import index_headers

        versions = [
            item.strip()
            for item in (args.versions or version).split(",")
            if item.strip()
        ]
        for selected in versions:
            print(index_headers(root, selected).relative_to(root))
        return 0
    if args.psyq_command == "scan":
        from ..psyq import scan_payload

        payloads = sorted(
            {
                *((root / "out" / "binaries").rglob("*.bin")),
                *((root / "out" / "extracted" / "BIN").rglob("*.bin")),
            }
        )
        rows = [
            {"path": str(path.relative_to(root)), "windows": len(scan_payload(path))}
            for path in payloads
        ]
        output = root / "out" / "index" / "psyq" / version / "scan.json"
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(
            json.dumps(
                {
                    "schema": "harness.psyq-scan/v1",
                    "version": version,
                    "payloads": rows,
                },
                indent=2,
                sort_keys=True,
            )
            + "\n",
            encoding="utf-8",
        )
        print(output.relative_to(root))
        return 0
    if args.psyq_command == "compare":
        left = args.left or "3.6"
        right = args.right or "4.0"
        rows = []
        for selected in (left, right):
            path = root / "out" / "index" / "psyq" / selected / "headers.json"
            if not path.is_file():
                from ..psyq import index_headers

                index_headers(root, selected)
                path = root / "out" / "index" / "psyq" / selected / "headers.json"
            payload = json.loads(path.read_text(encoding="utf-8"))
            rows.append(
                {
                    "version": selected,
                    "declarations": len(payload["declarations"]),
                    "types": len(payload["types"]),
                    "values": len(payload["values"]),
                }
            )
        print(
            json.dumps(
                {
                    "schema": "harness.psyq-compare/v1",
                    "left": rows[0],
                    "right": rows[1],
                },
                indent=2,
                sort_keys=True,
            )
        )
        return 0
    source = root / "toolchains" / "psyq" / version
    if args.psyq_command in {"extract", "convert"} and not source.is_dir():
        raise FileNotFoundError(f"PsyQ {version} is not staged at {source}")
    files = []
    if source.is_dir():
        for path in sorted(source.rglob("*")):
            if path.is_file():
                files.append(
                    {"path": str(path.relative_to(root)), "size": path.stat().st_size}
                )
    payload = {
        "schema": "harness.psyq-catalog/v1",
        "version": version,
        "command": args.psyq_command,
        "source": str(source.relative_to(root)),
        "files": files,
    }
    output = root / "out" / "index" / f"psyq_{version}.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    print(output.relative_to(root))
    return 0


def run_psyq_import_command(args: argparse.Namespace) -> int:
    from .toolchain import run_psyq_import

    return run_psyq_import(args)


def run_toolchain_probe(args: argparse.Namespace) -> int:
    from ..domain import load_profiles

    profiles = load_profiles(_root(args))
    rows = []
    for profile_id, profile in sorted(profiles.items()):
        rows.append(
            {
                "profile": profile_id,
                "compiler": profile.compiler,
                "assembler": profile.assembler,
                "linker": profile.linker,
                "runner": profile.runner,
                "status": "configured",
            }
        )
    print(json.dumps(rows, indent=2, sort_keys=True))
    return 0


def run_bench_workspace(args: argparse.Namespace) -> int:
    started = time.perf_counter()
    with tempfile.TemporaryDirectory(prefix="harness-") as directory:
        path = Path(directory) / "probe.bin"
        path.write_bytes(b"\0" * (1024 * 1024))
        _ = path.read_bytes()
    elapsed = time.perf_counter() - started
    print(
        json.dumps(
            {
                "workspace": tempfile.gettempdir(),
                "bytes": 1024 * 1024,
                "elapsed_seconds": round(elapsed, 6),
            },
            sort_keys=True,
        )
    )
    return 0


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
    from ..domain import load_target_manifests, normalize_target_id

    if args.target:
        candidate = args.target.split("@", 1)[0]
        try:
            target_id = normalize_target_id(candidate).value
        except ValueError:
            target_id = ""
        if target_id in load_target_manifests(_root(args)):
            from ..evidence import build_index, connect_index

            database = _root(args) / "out" / "index" / "harness.sqlite"
            if not database.is_file():
                build_index(_root(args), database)
            connection = connect_index(database)
            try:
                target = connection.execute(
                    "SELECT * FROM targets WHERE id = ?", (target_id,)
                ).fetchone()
                functions = connection.execute(
                    "SELECT COUNT(*) FROM functions WHERE target_id = ?", (target_id,)
                ).fetchone()[0]
            finally:
                connection.close()
            if target is None:
                raise ValueError(f"unknown target: {args.target}")
            print(
                f"{target_id}: {target['kind']}, profile={target['profile']}, functions={functions}"
            )
            return 0
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


def run_decomp_status(args: argparse.Namespace) -> int:
    from ..domain import normalize_target_id
    from ..evidence import build_index, connect_index

    root = _root(args)
    database = root / "out" / "index" / "harness.sqlite"
    build_index(root, database)
    connection = connect_index(database)
    try:
        parameters: tuple[str, ...] = ()
        where = ""
        if args.target:
            where = "WHERE target_id = ?"
            parameters = (normalize_target_id(args.target).value,)
        rows = connection.execute(
            f"SELECT target_id, address, source, behavior FROM functions {where} "
            "ORDER BY target_id, address",
            parameters,
        ).fetchall()
    finally:
        connection.close()

    files = []
    missing = 0
    for row in rows:
        source = row["source"]
        if not source:
            missing += 1
            continue
        source_path = Path(source)
        summary = (
            root
            / "out"
            / "matching"
            / str(source_path.parent).replace("/", "_")
            / source_path.stem
            / "summary.json"
        )
        match = read_json(summary) if summary.is_file() else {}
        percent = match.get("instruction_count", {}).get("match_percent")
        state = (
            "matched"
            if match.get("exact_match")
            else "different"
            if percent is not None
            else "unchecked"
        )
        files.append(
            {
                "target": row["target_id"],
                "address": f"0x{row['address']:08x}",
                "source": source,
                "status": state,
                "match_percent": percent,
            }
        )
    totals = Counter(row["status"] for row in files)
    payload = {
        "files": files,
        "totals": {**totals, "missing": missing, "total": len(rows)},
    }
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        for row in files:
            percent = (
                f" {row['match_percent']:.2f}%"
                if row["match_percent"] is not None
                else ""
            )
            print(f"{row['status'].upper():7s}{percent:8s} {row['source']}")
        print(
            "TOTAL "
            f"files={len(rows)} matched={totals['matched']} "
            f"different={totals['different']} unchecked={totals['unchecked']} "
            f"missing={missing}"
        )
    return 0


def run_inspect(args: argparse.Namespace) -> int:
    from ..binaries import resolve_entry, target_details

    catalog = read_json(_catalog_path(args))
    details = target_details(resolve_entry(catalog, args.target), _root(args))
    if args.json:
        print(json.dumps(details, indent=2, sort_keys=True))
        return 0
    print(f"target: {details['id']}")
    print(f"kind: {details['kind']}")
    print(f"status: {details['code_status']}")
    print(f"payload: {details['payload']}")
    print(f"sha256: {details['sha256']}")
    print(f"load address: 0x{details['load_address']:08x}")
    print(f"size: {details['size']} bytes")
    print(f"splat: {details['splat'] or '-'}")
    print(f"source: {details['source'] or '-'}")
    build = details["build"]
    if build:
        print(f"build target: {build['target']}")
        print(f"build stage: {build['stage']}")
        print(f"build output: {build['output'] or '-'}")
    else:
        print("build: not configured")
    progress = details["progress"]
    print(f"layout: {progress['layout']}")
    print(
        "functions: "
        f"reviewed={progress['reviewed_functions']} "
        f"lifted={progress['lifted_functions']} "
        f"matched={progress['matched_functions']}"
    )
    next_function = progress["next_function"]
    print(f"next function: {f'0x{next_function:08x}' if next_function else '-'}")
    print(f"whole payload match: {'yes' if progress['whole_payload_match'] else 'no'}")
    return 0


def run_next(args: argparse.Namespace) -> int:
    from ..binaries import resolve_entry, target_progress

    catalog = read_json(_catalog_path(args))
    rows = [
        entry for entry in catalog["entries"] if entry["code_status"] == "confirmed"
    ]
    if args.target:
        resolved = resolve_entry(catalog, args.target)
        rows = [entry for entry in rows if entry["id"] == resolved["id"]]
    if not rows:
        raise ValueError(
            "no confirmed target; run harness promote <archive#slot> --confirm-code first"
        )
    entry = sorted(rows, key=lambda item: item["id"])[0]
    next_function = target_progress(entry, _root(args))["next_function"]
    if next_function is None:
        raise ValueError(
            f"no unlifted reviewed function for {entry['id']}; review its Splat layout or inspect completion status"
        )
    print(f"harness lift {entry['id']}@0x{next_function:08x}")
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
    from ..domain import load_target_manifests, parse_function_id
    from ..match.asm_diff import AsmDiffRequest, run_asm_diff_one
    from ..match.asm_differ import write_bundle
    from ._asm_diff_output import format_asm_diff_summary

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
    else:
        print(format_asm_diff_summary(payload, root=root))
        if args.show_diff:
            print(Path(payload["outputs"]["diff"]).read_text(encoding="utf-8"))
    return 0 if payload["exact_match"] else 1


def run_flags(args: argparse.Namespace) -> int:
    from ..match.flag_search import search_flags

    payload = search_flags(
        layout=repo_layout(_root(args)),
        source=args.source,
        catalog_path=args.catalog
        or _root(args) / "config" / "compiler" / "flag-catalog.json",
    )
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        exact = payload["exact_matches"]
        for row in payload["results"]:
            flags = " ".join(row["flags"])
            print(f"{row['match_percent']:6.2f}%  {row['status']:13s}  {flags}")
        print(f"exact matches: {len(exact)}")
    return 0 if payload["exact_matches"] else 1


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

    write_json(output, {"schema": "harness.ghidra-imports/v1", "imports": imports})
    print(f"Ghidra import manifest: {output}")
    return 0


def run_analysis(args: argparse.Namespace) -> int:
    from ..analysis import doctor, export_project, initialize_project, query_project

    root = _root(args)
    if args.analysis_command == "doctor":
        payload = doctor()
    elif args.analysis_command == "init":
        payload = initialize_project(root, args.target, args.engine)
    elif args.analysis_command == "export":
        payload = export_project(root, args.target, args.engine)
    else:
        payload = query_project(root, args.target, args.query, args.engine)
    print(json.dumps(payload, indent=2, sort_keys=True))
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
    from ..domain import load_profiles, load_target_manifests

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
        else "configuration ready; run harness scan after extraction"
    )
    return 0


def build_parser() -> argparse.ArgumentParser:
    layout = repo_layout()
    parser = argparse.ArgumentParser(prog="harness")
    parser.add_argument("--root", type=Path, default=layout.root)
    sub = parser.add_subparsers(dest="command", required=True)
    target = sub.add_parser("target")
    target_sub = target.add_subparsers(dest="target_command", required=True)
    target_list = target_sub.add_parser("list")
    target_list.add_argument("--json", action="store_true")
    target_list.set_defaults(handler=run_target_list)
    target_show = target_sub.add_parser("show")
    target_show.add_argument("target")
    target_show.add_argument("--json", action="store_true")
    target_show.set_defaults(handler=run_target_show)
    target_promote = target_sub.add_parser("promote")
    target_promote.add_argument("target")
    target_promote.add_argument("--confirm-code", action="store_true")
    target_promote.set_defaults(handler=run_promote)
    target_doctor = target_sub.add_parser("doctor")
    target_doctor.add_argument("--strict", action="store_true")
    target_doctor.set_defaults(handler=run_doctor)

    index = sub.add_parser("index")
    index_sub = index.add_subparsers(dest="index_command", required=True)
    index_build = index_sub.add_parser("build")
    index_build.add_argument("--json", action="store_true")
    index_build.set_defaults(handler=run_index_build)
    index_find = index_sub.add_parser("find")
    index_find.add_argument("term", nargs="?")
    index_find.add_argument("--value")
    index_find.add_argument("--json", action="store_true")
    index_find.set_defaults(handler=run_index_find)
    index_show = index_sub.add_parser("show")
    index_show.add_argument("identifier")
    index_show.add_argument("--json", action="store_true")
    index_show.set_defaults(handler=run_index_show)
    index_related = index_sub.add_parser("related")
    index_related.add_argument("identifier")
    index_related.set_defaults(handler=run_index_related)

    find = sub.add_parser("find")
    find.add_argument("term", nargs="?")
    find.add_argument("--value")
    find.add_argument("--json", action="store_true")
    find.set_defaults(handler=run_index_find)
    show = sub.add_parser("show")
    show.add_argument("identifier")
    show.add_argument("--json", action="store_true")
    show.set_defaults(handler=run_index_show)
    related = sub.add_parser("related")
    related.add_argument("identifier")
    related.set_defaults(handler=run_index_related)
    graph = sub.add_parser("graph")
    graph.add_argument("identifier")
    graph.add_argument("--depth", type=int, default=2)
    graph.add_argument("--json", action="store_true")
    graph.set_defaults(handler=run_graph)
    path = sub.add_parser("path")
    path.add_argument("left")
    path.add_argument("right")
    path.add_argument("--json", action="store_true")
    path.set_defaults(handler=run_path)

    profile = sub.add_parser("profile")
    profile_sub = profile.add_subparsers(dest="profile_command", required=True)
    profile_list = profile_sub.add_parser("list")
    profile_list.add_argument("--json", action="store_true")
    profile_list.set_defaults(handler=run_profile)
    profile_show = profile_sub.add_parser("show")
    profile_show.add_argument("profile")
    profile_show.set_defaults(handler=run_profile)
    profile_resolve = profile_sub.add_parser("resolve")
    profile_resolve.add_argument("target")
    profile_resolve.add_argument("--compiler")
    profile_resolve.add_argument("--assembler")
    profile_resolve.add_argument("--headers")
    profile_resolve.add_argument("--objects")
    profile_resolve.add_argument("--linker")
    profile_resolve.add_argument("--runner")
    profile_resolve.set_defaults(handler=run_profile)

    psyq = sub.add_parser("psyq")
    psyq_sub = psyq.add_subparsers(dest="psyq_command", required=True)
    psyq_import = psyq_sub.add_parser("import")
    psyq_import.add_argument("--version", default="4.7")
    psyq_import.add_argument("--archive", type=Path)
    psyq_import.add_argument("--archive-url")
    psyq_import.add_argument("--dest", type=Path)
    psyq_import.add_argument("--private-root", type=Path)
    psyq_import.add_argument("--force", action="store_true")
    psyq_import.set_defaults(handler=run_psyq_import_command)
    for command in (
        "catalog",
        "extract",
        "convert",
        "validate",
        "index",
        "compare",
        "scan",
        "report",
    ):
        command_parser = psyq_sub.add_parser(command)
        command_parser.add_argument("--version", default="4.7")
        command_parser.add_argument("--versions")
        command_parser.add_argument("--left")
        command_parser.add_argument("--right")
        command_parser.set_defaults(handler=run_psyq_inventory)

    toolchain = sub.add_parser("toolchain")
    toolchain_sub = toolchain.add_subparsers(dest="toolchain_command", required=True)
    for command in ("probe", "compare-probes", "matrix"):
        toolchain_command = toolchain_sub.add_parser(command)
        toolchain_command.set_defaults(handler=run_toolchain_probe)

    bench = sub.add_parser("bench")
    bench_sub = bench.add_subparsers(dest="bench_command", required=True)
    bench_workspace = bench_sub.add_parser("workspace")
    bench_workspace.set_defaults(handler=run_bench_workspace)

    context = sub.add_parser("context")
    context_sub = context.add_subparsers(dest="context_command", required=True)
    context_build = context_sub.add_parser("build")
    context_build.add_argument("target")
    context_build.add_argument("function", help="hex or decimal function address")
    context_build.add_argument(
        "--mode", choices=("prelude", "minimal", "full"), default="prelude"
    )
    context_build.set_defaults(handler=run_context_build)
    context_show = context_sub.add_parser("show")
    context_show.add_argument("target")
    context_show.set_defaults(handler=run_context_show)

    accept = sub.add_parser("accept")
    accept.add_argument("candidate", type=Path)
    accept.add_argument("--target", required=True)
    accept.add_argument("--function", required=True)
    accept.set_defaults(handler=run_accept)

    permute = sub.add_parser("permute")
    permute.add_argument("source")
    permute.add_argument("--work-root", type=Path)
    permute.add_argument("--prepare-only", action="store_true")
    permute.add_argument("--quiet", action="store_true")
    permute.add_argument("--silent", action="store_true")
    permute.add_argument("--verbose", action="store_true")
    permute.add_argument("--show-errors", action="store_true")
    permute.add_argument("--show-timings", action="store_true")
    permute.add_argument("-j", "--jobs", type=int)
    permute.set_defaults(handler=run_permute)
    adopt = sub.add_parser("adopt")
    adopt.add_argument("candidate", type=Path)
    adopt.add_argument("--function")
    adopt.add_argument("--apply", action="store_true")
    adopt.add_argument("--allow-nonmatch", action="store_true")
    adopt.set_defaults(handler=run_adopt)

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
    decomp_status = sub.add_parser("decomp-status")
    decomp_status.add_argument("target", nargs="?")
    decomp_status.add_argument("--json", action="store_true")
    decomp_status.set_defaults(handler=run_decomp_status)
    inspect = sub.add_parser("inspect")
    inspect.add_argument("target")
    inspect.add_argument("--json", action="store_true")
    inspect.set_defaults(handler=run_inspect)
    nxt = sub.add_parser("next")
    nxt.add_argument("target", nargs="?")
    nxt.set_defaults(handler=run_next)
    lift = sub.add_parser("lift")
    lift.add_argument("target")
    lift.add_argument("function", nargs="?")
    lift.add_argument("--seed")
    lift.set_defaults(handler=run_lift_workflow)
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
    diff.add_argument("--json", action="store_true")
    diff.add_argument("--show", "--show-diff", dest="show_diff", action="store_true")
    diff.add_argument("--html", action="store_true")
    diff.add_argument("--watch", action="store_true")
    diff.set_defaults(handler=run_diff)
    flags = sub.add_parser("flags")
    flags.add_argument("source", type=Path)
    flags.add_argument(
        "--catalog",
        type=Path,
    )
    flags.add_argument("--json", action="store_true")
    flags.set_defaults(handler=run_flags)
    ghidra = sub.add_parser("ghidra")
    ghidra_sub = ghidra.add_subparsers(dest="ghidra_command", required=True)
    ghidra_sync = ghidra_sub.add_parser("sync")
    ghidra_sync.set_defaults(handler=run_ghidra_sync)
    ghidra_export = ghidra_sub.add_parser("export")
    ghidra_export.set_defaults(handler=run_ghidra_sync)
    analysis = sub.add_parser("analysis")
    analysis_sub = analysis.add_subparsers(dest="analysis_command", required=True)
    analysis_doctor = analysis_sub.add_parser("doctor")
    analysis_doctor.set_defaults(handler=run_analysis)
    for command in ("init", "export"):
        analysis_command = analysis_sub.add_parser(command)
        analysis_command.add_argument("target")
        analysis_command.add_argument("--engine", choices=("rizin", "r2"))
        analysis_command.set_defaults(handler=run_analysis)
    analysis_query = analysis_sub.add_parser("query")
    analysis_query.add_argument("target")
    analysis_query.add_argument(
        "query", help="functions, strings, xrefs, types, or a JSON command"
    )
    analysis_query.add_argument("--engine", choices=("rizin", "r2"))
    analysis_query.set_defaults(handler=run_analysis)
    assets = sub.add_parser("assets")
    assets_sub = assets.add_subparsers(dest="assets_command", required=True)
    assets_list = assets_sub.add_parser("list")
    assets_list.set_defaults(handler=run_assets_list)
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
