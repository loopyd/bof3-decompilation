"""No-follow, stable input inventory for read-only decompilation coverage."""

from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import stat
import tomllib

import yaml

from ..domain.manifests import load_target_manifests


def checked_path(root: Path, relative: str = "") -> Path:
    if not isinstance(relative, str):
        raise ValueError(f"input path must be a string: {relative!r}")
    if relative and (
        Path(relative).is_absolute()
        or any(part in {"", ".", ".."} for part in relative.split("/"))
        or "\\" in relative
    ):
        raise ValueError(f"unsafe input path: {relative!r}")
    path = root / relative
    for ancestor in reversed((path, *path.parents)):
        mode = ancestor.lstat().st_mode
        if stat.S_ISLNK(mode) or not (stat.S_ISDIR(mode) or stat.S_ISREG(mode)):
            raise ValueError(f"unsafe input type: {ancestor}")
    return path


class CoverageInputs:
    """Retain observed identities and recheck them before returning stdout."""

    def __init__(self, root: Path):
        self.root = root.absolute()
        checked_path(self.root)
        self.files: dict[str, dict] = {}
        self.directories: dict[str, list[str]] = {}
        self.missing: set[str] = set()

    def read(self, relative: str) -> bytes:
        try:
            path = checked_path(self.root, relative)
        except FileNotFoundError:
            self.missing.add(relative)
            raise
        before = path.stat()
        if not stat.S_ISREG(before.st_mode):
            raise ValueError(f"not a regular input: {relative}")
        with path.open("rb") as stream:
            data = stream.read()
            after = os.fstat(stream.fileno())

        def identity(s):
            return (
                s.st_dev,
                s.st_ino,
                s.st_mode,
                s.st_size,
                s.st_mtime_ns,
                s.st_ctime_ns,
            )

        if identity(before) != identity(after) or identity(path.stat()) != identity(
            before
        ):
            raise ValueError(f"moving input: {relative}")
        row = {
            "bytes": len(data),
            "sha256": hashlib.sha256(data).hexdigest(),
            "mode": stat.S_IMODE(before.st_mode),
        }
        if relative in self.files and self.files[relative] != row:
            raise ValueError(f"changed input: {relative}")
        self.files[relative] = row
        return data

    def walk(self, relative: str) -> list[str]:
        directory = checked_path(self.root, relative)
        names = sorted(p.name for p in directory.iterdir())
        if relative in self.directories and self.directories[relative] != names:
            raise ValueError(f"changed directory: {relative}")
        self.directories[relative] = names
        files = []
        for name in names:
            child = f"{relative}/{name}"
            path = checked_path(self.root, child)
            files.extend(self.walk(child) if path.is_dir() else [child])
        return files

    def verify(self) -> None:
        for relative in list(self.files):
            self.read(relative)
        for relative, names in self.directories.items():
            path = checked_path(self.root, relative)
            if sorted(p.name for p in path.iterdir()) != names:
                raise ValueError(f"changed directory: {relative}")
        for relative in self.missing:
            if os.path.lexists(self.root / relative):
                raise ValueError(f"previously missing input appeared: {relative}")


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON member: {key}")
        result[key] = value
    return result


def inventory_archives(inputs: CoverageInputs) -> dict:
    files = inputs.walk("out/extracted/BIN")
    archives = sorted(p for p in files if p.endswith(".EMI"))
    manifests = {p for p in files if p.endswith("/emi.json")}
    expected = {p[:-4] + "/emi.json" for p in archives}
    errors = [f"unowned EMI manifest: {p}" for p in sorted(manifests - expected)]
    slots, payloads = [], set()
    declared, parsed, complete = 0, 0, True
    for archive in archives:
        inputs.read(archive)
        manifest = archive[:-4] + "/emi.json"
        try:
            document = json.loads(
                inputs.read(manifest), object_pairs_hook=unique_object
            )
            if not isinstance(document, dict) or not isinstance(
                document.get("entries"), list
            ):
                raise ValueError("entries must be an array")
            entries = document["entries"]
        except (OSError, ValueError) as exc:
            errors.append(f"{manifest}: {exc}")
            complete = False
            continue
        declared += len(entries)
        seen = set()
        for position, entry in enumerate(entries):
            try:
                if not isinstance(entry, dict):
                    raise ValueError("entry must be an object")
                for key in ("index", "size", "ram_ptr", "type"):
                    if (
                        type(entry.get(key)) is not int
                        or not 0 <= entry[key] <= 0xFFFFFFFF
                    ):
                        raise ValueError(f"invalid entry {key}")
                name = entry.get("name")
                if (
                    not isinstance(name, str)
                    or not name
                    or name in {".", ".."}
                    or "/" in name
                    or "\\" in name
                ):
                    raise ValueError(f"invalid payload name: {name!r}")
                parsed += 1
                index = entry["index"]
                path = archive[:-4] + "/" + name
                if index in seen or path in payloads:
                    raise ValueError(f"duplicate slot/payload: {index}: {path}")
                seen.add(index)
                payloads.add(path)
                row = {
                    "identity": archive.removeprefix("out/extracted/") + f"#{index}",
                    "path": path,
                    "index": index,
                    "type": entry["type"],
                    "load_address": entry["ram_ptr"],
                    "declared_size": entry["size"],
                }
                slots.append(row)
                try:
                    data = inputs.read(path)
                    row.update(inputs.files[path])
                    if len(data) != entry["size"]:
                        raise ValueError(f"payload size mismatch: {path}")
                except (OSError, ValueError) as exc:
                    row["error"] = str(exc)
                    errors.append(f"{path}: {exc}")
            except ValueError as exc:
                errors.append(f"{manifest} entry {position}: {exc}")
    orphans = sorted(set(files) - set(archives) - manifests - payloads)
    for path in orphans:
        inputs.read(path)
        errors.append(f"unmanifested payload: {path}")
    return {
        "archives": archives,
        "archive_count": len(archives),
        "manifest_count": len(manifests),
        "slots": slots,
        "declared_slot_count": declared if complete else None,
        "observed_declaration_count": declared,
        "declarations_complete": complete,
        "parsed_slot_count": parsed,
        "unique_slot_count": len(slots),
        "valid_slot_count": sum("error" not in row for row in slots),
        "orphan_payloads": orphans,
        "discrepancies": errors,
    }


def _layout_integer(value):
    if type(value) is int:
        return
    if isinstance(value, str):
        try:
            int(value, 0)
            return
        except ValueError:
            pass
    raise ValueError(f"invalid Splat integer: {value!r}")


def _layout_row(row, *, terminal=False):
    if not isinstance(row, list) or not row:
        raise ValueError("invalid Splat row")
    _layout_integer(row[0])
    if terminal and len(row) == 1:
        return
    if len(row) < 2 or not isinstance(row[1], str):
        raise ValueError("invalid Splat row kind")
    metadata = row[2] if len(row) > 2 else None
    if metadata is not None and not isinstance(metadata, (str, dict)):
        raise ValueError("invalid Splat metadata")
    if isinstance(metadata, dict):
        for key in ("name", "source", "behavior"):
            if metadata.get(key) is not None and not isinstance(metadata[key], str):
                raise ValueError(f"invalid Splat {key}")
    if any(not isinstance(extra, str) for extra in row[3:]):
        raise ValueError("invalid Splat metadata extras")


def read_layout_document(inputs, path):
    try:
        return _read_layout_document(inputs, path)
    except (ValueError, yaml.YAMLError) as exc:
        raise ValueError(f"{path}: invalid Splat input: {exc}") from exc


def _read_layout_document(inputs, path):
    document = yaml.safe_load(inputs.read(path).decode())
    if not isinstance(document, dict) or not isinstance(document.get("segments"), list):
        raise ValueError("invalid Splat document/segments")
    for segment in document["segments"]:
        if isinstance(segment, list):
            _layout_row(segment, terminal=True)
        elif isinstance(segment, dict):
            for key in ("start", "vram"):
                if key in segment:
                    _layout_integer(segment[key])
            rows = segment.get("subsegments", [])
            if not isinstance(rows, list):
                raise ValueError("invalid Splat subsegments")
            for row in rows:
                _layout_row(row)
        else:
            raise ValueError("invalid Splat segment")
    options = document.get("options", {})
    if not isinstance(options, dict):
        raise ValueError("invalid Splat options")
    maps = options.get("symbol_addrs_path", [])
    maps = [maps] if isinstance(maps, str) else maps
    if not isinstance(maps, list) or not all(isinstance(p, str) and p for p in maps):
        raise ValueError("invalid Splat symbol_addrs_path")
    for selected in maps:
        inputs.read(selected)
    return document


def read_manifests(inputs: CoverageInputs):
    paths = [p for p in inputs.walk("config/targets") if p.endswith(".toml")]
    seen = set()
    for path in paths:
        try:
            raw = tomllib.loads(inputs.read(path).decode())
        except ValueError as exc:
            raise ValueError(f"invalid manifest {path}: {exc}") from exc
        _validate_companion_types(raw.get("companion_overlays", []), path)
        for key in ("id", "kind", "source_dir", "binary", "splat"):
            if not isinstance(raw.get(key), str) or not raw[key]:
                raise ValueError(f"invalid manifest {key}: {path}")
        if type(raw.get("load_address", 0)) is not int:
            raise ValueError(f"invalid manifest load_address: {path}")
        for key in ("psyq", "matching"):
            if not isinstance(raw.get(key, {}), dict):
                raise ValueError(f"invalid manifest {key}: {path}")
        if not isinstance(raw.get("psyq", {}).get("space", "slus"), str):
            raise ValueError(f"invalid manifest psyq space: {path}")
        libraries = raw.get("psyq", {}).get("libraries", {})
        if not isinstance(libraries, dict):
            raise ValueError("invalid manifest libraries")
        for library in libraries.values():
            if not isinstance(library, dict):
                raise ValueError("invalid manifest library")
            if not isinstance(library.get("confidence", ""), str):
                raise ValueError(f"invalid library confidence: {path}")
            for key in ("members", "evidence"):
                if not isinstance(library.get(key, []), list) or not all(
                    isinstance(v, str) for v in library.get(key, [])
                ):
                    raise ValueError(f"invalid library {key}")
        placements = raw.get("matching", {}).get("section_placements", [])
        if not isinstance(placements, list):
            raise ValueError("invalid section placements")
        for placement in placements:
            if not isinstance(placement, dict) or not isinstance(
                placement.get("section"), str
            ):
                raise ValueError("invalid section placement")
            if any(
                type(placement.get(k)) is not int
                for k in ("function", "address", "size")
            ):
                raise ValueError("invalid section placement integers")
        if "disc_id" in raw and not isinstance(raw["disc_id"], str):
            raise ValueError("invalid manifest disc_id")
        identity = raw.get("id")
        if identity in seen:
            raise ValueError(f"duplicate target manifest: {identity}")
        seen.add(identity)
        # Validate caller spelling before the manifest owner's resolving cache.
        for key in ("source_dir", "binary", "splat", "psyq_source"):
            value = raw.get(key, "")
            if not isinstance(value, str):
                raise ValueError(f"invalid manifest {key}: {path}")
            if value:
                try:
                    if key == "source_dir":
                        if not checked_path(inputs.root, value).is_dir():
                            raise ValueError(f"not a directory input: {value}")
                    else:
                        inputs.read(value)
                except FileNotFoundError:
                    pass
        for key in ("sources", "support_sources", "headers"):
            claims = raw.get(key, [])
            if not isinstance(claims, list) or not all(
                isinstance(v, str) and v for v in claims
            ):
                raise ValueError(f"invalid manifest {key}")
            for claimed in claims:
                inputs.read(claimed)
    return load_target_manifests(inputs.root)


def _validate_companion_types(values, path):
    if not isinstance(values, list):
        raise ValueError(f"invalid manifest companion_overlays: {path}")
    for value in values:
        _require_fields(
            value,
            ("target", "disc_id", "payload_sha256", "evidence"),
            ("load_address", "size"),
            path,
        )
        calls = value.get("static_calls", [])
        if not isinstance(calls, list):
            raise ValueError(f"invalid manifest static_calls: {path}")
        for call in calls:
            _require_fields(call, (), ("caller_address", "target_address"), path)
        if "abi" in value:
            _require_fields(
                value["abi"], ("prototype", "evidence"), ("target_address",), path
            )


def _require_fields(value, strings, integers, path):
    if not isinstance(value, dict):
        raise ValueError(f"invalid manifest table: {path}")
    for key in strings:
        if not isinstance(value.get(key), str):
            raise ValueError(f"invalid manifest {key}: {path}")
    for key in integers:
        if type(value.get(key)) is not int:
            raise ValueError(f"invalid manifest {key}: {path}")
