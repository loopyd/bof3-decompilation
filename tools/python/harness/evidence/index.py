"""Build and query the generated ``harness.sqlite`` evidence graph.

The database is an index, never an authored source of binary truth.  Every
record that points outside SQLite retains the source path and a content hash.
"""

from __future__ import annotations

import hashlib
import json
import re
import sqlite3
from pathlib import Path
from typing import Any, Iterable

from ..domain import load_profiles, load_target_manifests, normalize_target_id
from ..jsonio import read_json, write_json
from ..symbols import parse_weak_symbol_bindings
from .schema import _table_sql, connect, create_schema


SCHEMA_VERSION = "harness.evidence/v1"
FUNCTION_RE = re.compile(r"func_([0-9a-fA-F]{8})\.c$")
CALL_RE = re.compile(r"\b(func_[0-9a-fA-F]{8})\s*\(")


def _sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def connect_index(path: Path) -> sqlite3.Connection:
    """Open a generated index for read-only or query use."""

    return connect(path)


graph_schema = create_schema


def _catalog_entries(root: Path) -> Iterable[dict[str, Any]]:
    path = root / "out" / "catalog" / "emi.json"
    if not path.is_file():
        return ()
    payload = read_json(path)
    return payload.get("entries", ())


def _function_records(
    root: Path, target_id: str, source_dir: str
) -> Iterable[dict[str, Any]]:
    directory = root / source_dir
    if not directory.is_dir():
        return ()
    records: list[dict[str, Any]] = []
    for path in sorted(directory.glob("func_*.c")):
        match = FUNCTION_RE.match(path.name)
        if match is None:
            continue
        text = path.read_text(encoding="utf-8")
        behavior = ""
        for line in text.splitlines():
            if "@behavior" in line:
                behavior = line.split("@behavior", 1)[1].strip()
                break
        records.append(
            {
                "id": f"{target_id}@{int(match.group(1), 16):08x}",
                "target_id": target_id,
                "address": int(match.group(1), 16),
                "source": str(path.relative_to(root)),
                "source_sha256": bytes.fromhex(_sha256(path)),
                "behavior": behavior,
            }
        )
    return records


def build_index(root: Path, database: Path | None = None) -> dict[str, Any]:
    """Rebuild the deterministic graph and return its compact summary."""

    root = root.resolve()
    database = database or root / "out" / "index" / "harness.sqlite"
    manifests = load_target_manifests(root)
    profiles = load_profiles(root)
    from .repository import EvidenceRepository

    with EvidenceRepository(database) as repository:
        repository.initialize()
        repository.reset()
        repository.execute(
            "INSERT INTO metadata(key, value) VALUES (?, ?)", ("schema", SCHEMA_VERSION)
        )
        repository.execute(
            "INSERT INTO metadata(key, value) VALUES (?, ?)", ("root", str(root))
        )
        function_sources: dict[str, str] = {}
        function_ids: set[str] = set()
        symbol_ids: dict[str, str] = {}
        payload_hashes: dict[str, list[str]] = {}
        entry_hashes: dict[str, list[str]] = {}
        for target_id, manifest in sorted(manifests.items()):
            repository.insert(
                "targets",
                {
                    "id": target_id,
                    "kind": manifest.kind,
                    "disc_id": manifest.disc_id,
                    "source_dir": manifest.source_dir,
                    "binary": manifest.binary,
                    "splat": manifest.splat,
                    "load_address": manifest.load_address,
                    "profile": manifest.profile,
                },
            )
            for library_name, members in sorted(manifest.libraries.items()):
                library_id = f"{target_id}:psyq:{library_name}"
                repository.insert(
                    "psyq_libraries",
                    {
                        "id": library_id,
                        "version_id": manifest.profile,
                        "name": library_name,
                    },
                    ignore=True,
                )
                repository.edge(target_id, "USES_LIBRARY", library_id)
                confidence = manifest.library_confidence.get(library_name)
                evidence = manifest.library_evidence.get(library_name, ())
                for member_name in members:
                    member_path = Path(member_name)
                    if member_path.parts[:1] == ("psyq",):
                        member_path = Path(*member_path.parts[1:])
                    version = {
                        "original/psyq36": "3.6",
                        "original/psyq40": "4.0",
                        "native/capcom97": "4.7",
                    }.get(manifest.profile, manifest.profile)
                    staged_member = root / "toolchains" / "psyq" / version / member_path
                    member_id = f"{library_id}:{member_name}"
                    repository.insert(
                        "psyq_members",
                        {
                            "id": member_id,
                            "library_id": library_id,
                            "path": member_name,
                            "sha256": (
                                bytes.fromhex(_sha256(staged_member))
                                if staged_member.is_file()
                                else None
                            ),
                        },
                        ignore=True,
                    )
                    repository.edge(library_id, "CONTAINS", member_id)
                if confidence or evidence:
                    repository.insert(
                        "evidence",
                        {
                            "id": f"{library_id}:selection",
                            "subject_id": target_id,
                            "kind": "psyq-library-selection",
                            "strength": {
                                "high": 1.0,
                                "medium": 0.6,
                                "low": 0.3,
                            }.get(confidence or "", 0.0),
                            "detail": json.dumps(
                                {
                                    "library": library_name,
                                    "members": list(members),
                                    "evidence": list(evidence),
                                },
                                sort_keys=True,
                            ),
                        },
                        ignore=True,
                    )
            binary = root / manifest.binary
            if binary.is_file():
                digest = _sha256(binary)
                artifact_id = f"{target_id}::payload"
                repository.insert(
                    "artifacts",
                    {
                        "id": artifact_id,
                        "kind": "payload",
                        "path": manifest.binary,
                        "sha256": bytes.fromhex(digest),
                        "provenance": manifest.disc_id,
                    },
                )
                repository.insert(
                    "fingerprints",
                    {
                        "id": artifact_id,
                        "subject_id": target_id,
                        "kind": "payload-sha256",
                        "value": bytes.fromhex(digest),
                    },
                )
                repository.edge(target_id, "HAS_ARTIFACT", artifact_id)
                payload_hashes.setdefault(digest, []).append(target_id)
            for function in _function_records(root, target_id, manifest.source_dir):
                repository.insert("functions", function)
                repository.edge(target_id, "CONTAINS", function["id"])
                repository.insert(
                    "fingerprints",
                    {
                        "id": f"{function['id']}::source",
                        "subject_id": function["id"],
                        "kind": "source-sha256",
                        "value": function["source_sha256"],
                    },
                )
                function_ids.add(function["id"])
                function_sources[function["id"]] = (
                    root / function["source"]
                ).read_text(encoding="utf-8")
            symbol_source = root / manifest.source_dir / "symbols.c"
            if symbol_source.is_file():
                symbol_text = symbol_source.read_text(encoding="utf-8")
                for name, address in parse_weak_symbol_bindings(symbol_text).items():
                    symbol_id = f"{target_id}::{name}"
                    symbol_ids[name] = symbol_id
                    repository.insert(
                        "symbols",
                        {
                            "id": symbol_id,
                            "name": name,
                            "address": address,
                            "target_id": target_id,
                        },
                    )
                    repository.edge(target_id, "CONTAINS", symbol_id)
        for function_id, source_text in sorted(function_sources.items()):
            target_id = function_id.split("@", 1)[0]
            for name in sorted(set(CALL_RE.findall(source_text))):
                callee_id = f"{target_id}@{name.removeprefix('func_').lower()}"
                if callee_id not in function_ids or callee_id == function_id:
                    continue
                repository.execute(
                    "INSERT OR IGNORE INTO calls(caller_id, callee_id) VALUES (?, ?)",
                    (function_id, callee_id),
                )
                repository.edge(function_id, "CALLS", callee_id)
            for name, symbol_id in sorted(symbol_ids.items()):
                if re.search(rf"\b{re.escape(name)}\b", source_text):
                    repository.execute(
                        'INSERT OR IGNORE INTO "references"(function_id, symbol_id) VALUES (?, ?)',
                        (function_id, symbol_id),
                    )
                    repository.edge(function_id, "REFERENCES", symbol_id)
        for entry in _catalog_entries(root):
            entry_id = str(entry["id"])
            container_id = str(entry.get("archive_id", ""))
            if container_id:
                repository.execute(
                    "INSERT OR IGNORE INTO containers (id, path, sha256) VALUES (?, ?, ?)",
                    (container_id, str(entry.get("archive_id", "")), None),
                )
                repository.edge(container_id, "CONTAINS", entry_id)
            repository.insert(
                "entries",
                {
                    "id": entry_id,
                    "container_id": container_id or None,
                    "slot": entry.get("slot"),
                    "path": entry.get("payload_path"),
                    "sha256": bytes.fromhex(str(entry["sha256"])),
                    "load_address": entry.get("load_address"),
                    "size": entry.get("size"),
                    "payload_kind": entry.get("payload_kind"),
                    "code_status": entry.get("code_status"),
                },
            )
            entry_hashes.setdefault(str(entry["sha256"]), []).append(entry_id)
            try:
                target_id = normalize_target_id(entry_id).value
            except ValueError:
                target_id = ""
            if target_id in manifests:
                repository.edge(entry_id, "NORMALIZES_TO", target_id)
        for digest, target_ids in sorted(payload_hashes.items()):
            if len(target_ids) < 2:
                continue
            group_id = f"payload:{digest}"
            repository.insert(
                "duplicate_groups",
                {
                    "id": group_id,
                    "kind": "payload-sha256",
                    "fingerprint": bytes.fromhex(digest),
                },
            )
            for target_id in target_ids:
                repository.edge(group_id, "SAME_BYTES", target_id)
        for digest, entry_ids in sorted(entry_hashes.items()):
            if len(entry_ids) < 2:
                continue
            group_id = f"entry-payload:{digest}"
            repository.insert(
                "duplicate_groups",
                {
                    "id": group_id,
                    "kind": "entry-payload-sha256",
                    "fingerprint": bytes.fromhex(digest),
                },
                ignore=True,
            )
            for entry_id in entry_ids:
                repository.edge(group_id, "SAME_BYTES", entry_id)
        for profile_id, profile in sorted(profiles.items()):
            version = {
                "original/psyq36": "3.6",
                "original/psyq40": "4.0",
                "native/capcom97": "4.7",
            }.get(profile_id, profile_id)
            repository.insert(
                "psyq_versions",
                {
                    "id": profile_id,
                    "version": version,
                    "source": profile.compiler,
                    "sha256": None,
                },
            )
            lib_root = root / "toolchains" / "psyq" / version / "lib"
            if lib_root.is_dir():
                for library_dir in sorted(
                    path for path in lib_root.iterdir() if path.is_dir()
                ):
                    library_id = f"{profile_id}:{library_dir.name}"
                    repository.insert(
                        "psyq_libraries",
                        {
                            "id": library_id,
                            "version_id": profile_id,
                            "name": library_dir.name,
                        },
                        ignore=True,
                    )
                    for member in sorted(
                        path for path in library_dir.rglob("*") if path.is_file()
                    ):
                        member_id = (
                            f"{library_id}:{member.relative_to(lib_root).as_posix()}"
                        )
                        repository.insert(
                            "psyq_members",
                            {
                                "id": member_id,
                                "library_id": library_id,
                                "path": str(member.relative_to(root)),
                                "sha256": bytes.fromhex(_sha256(member)),
                            },
                            ignore=True,
                        )
                        repository.edge(library_id, "CONTAINS", member_id)
            version_root = root / "toolchains" / "psyq" / version / "include"
            if version_root.is_dir():
                from ..psyq import parse_headers

                header_graph = parse_headers(version_root)
                for item in header_graph["types"]:
                    type_id = f"{version}:{item['id']}"
                    repository.insert(
                        "types",
                        {
                            "id": type_id,
                            "name": item["name"],
                            "layout_hash": bytes.fromhex(item["layout_hash"]),
                        },
                        ignore=True,
                    )
                    repository.edge(profile_id, "USES_TYPE", type_id)
                for item in header_graph["fields"]:
                    field_id = f"{version}:{item['id']}"
                    repository.insert(
                        "fields",
                        {
                            "id": f"{version}:{item['id']}",
                            "type_id": f"{version}:{item['type_id']}",
                            "name": item["name"],
                            "offset": item["offset"],
                            "field_type": item["field_type"],
                        },
                        ignore=True,
                    )
                    repository.edge(
                        f"{version}:{item['type_id']}", "HAS_FIELD", field_id
                    )
                for item in header_graph["declarations"]:
                    repository.insert(
                        "declarations",
                        {
                            "id": f"{version}:{item['id']}",
                            "name": item["name"],
                            "kind": item["kind"],
                            "source": item["source"],
                        },
                        ignore=True,
                    )
                for item in header_graph["values"]:
                    repository.insert(
                        "values",
                        {
                            "id": f"{version}:{item['id']}",
                            "name": item["name"],
                            "value": item["value"],
                            "declaration_id": None,
                        },
                        ignore=True,
                    )
        summary = {
            "schema": SCHEMA_VERSION,
            "database": str(database.relative_to(root)),
            "targets": len(manifests),
            "profiles": len(profiles),
            "entries": repository.execute("SELECT COUNT(*) FROM entries").fetchone()[0],
            "functions": repository.execute(
                "SELECT COUNT(*) FROM functions"
            ).fetchone()[0],
            "edges": repository.execute("SELECT COUNT(*) FROM edges").fetchone()[0],
        }
    out_dir = root / "out" / "index"
    write_json(out_dir / "summary.json", summary)
    (out_dir / "summary.md").write_text(
        "# Harness evidence index\n\n"
        f"Targets: {summary['targets']}  \nProfiles: {summary['profiles']}  \n"
        f"Entries: {summary['entries']}  \nFunctions: {summary['functions']}  \n",
        encoding="utf-8",
    )
    return summary


def find_records(connection: sqlite3.Connection, term: str) -> list[dict[str, Any]]:
    """Find targets, functions, symbols, values, and declarations by text."""

    like = f"%{term}%"
    rows: list[dict[str, Any]] = []
    for table, columns in (
        ("targets", ("id", "disc_id")),
        ("functions", ("id", "source", "behavior")),
        ("symbols", ("id", "name")),
        ("types", ("id", "name")),
        ("fields", ("id", "name", "field_type")),
        ("values", ("id", "name", "value")),
        ("declarations", ("id", "name")),
    ):
        predicate = " OR ".join(f"{column} LIKE ?" for column in columns)
        for row in connection.execute(
            f"SELECT * FROM {_table_sql(table)} WHERE {predicate}",
            (like,) * len(columns),
        ):
            item = dict(row)
            for key, value in tuple(item.items()):
                if isinstance(value, bytes):
                    item[key] = value.hex()
            item["table"] = table
            rows.append(item)
    return rows
