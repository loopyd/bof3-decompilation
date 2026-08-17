"""EMI target bootstrap: plan, apply, and materialize reviewed targets."""

from __future__ import annotations

import hashlib
import json
import os
import tempfile
from pathlib import Path
from typing import Any

from ..domain.symbols import load_map
from ..discovery import file_sha256
from ..domain import load_target_manifests
from ..io import write_json

from .catalog_verify import resolve_entry, target_slug, verify_companion_relations


def bootstrap_plan(
    root: Path, catalog: dict[str, Any], identifier: str
) -> dict[str, Any]:
    entry = resolve_entry(catalog, identifier)
    reasons = _eligibility(root, entry)
    if reasons:
        raise ValueError(f"ineligible EMI entry {entry['id']}: {', '.join(reasons)}")
    slug = target_slug(entry)
    target = f"emi/{slug}"
    binary = f"out/binaries/emi/{slug}.bin"
    manifest = f"config/targets/emi/{slug}/target.toml"
    splat = f"config/targets/emi/{slug}/splat.yaml"
    symbols = f"config/targets/emi/{slug}/symbols.txt"
    basename = slug.replace("/", "_")
    payload = Path(entry["payload_path"])
    source = payload.relative_to(root).as_posix()
    metadata = (
        json.dumps(
            {
                "schema": "harness.normalized-emi/v1",
                "source": source,
                "source_sha256": entry["sha256"],
                "image": binary,
                "image_sha256": entry["sha256"],
                "load_address": entry["load_address"],
            },
            indent=2,
            sort_keys=True,
        )
        + "\n"
    )
    manifest_text = (
        'schema = "harness.target/v2"\n'
        f'id = "{target}"\n'
        f'disc_id = "BIN/{entry["archive_id"].upper()}.EMI#{entry["slot"]}"\n'
        'kind = "emi"\n'
        f'source_dir = "src/emi/{slug}"\n'
        f'binary = "{binary}"\n'
        f'splat = "{splat}"\n'
        f"load_address = 0x{entry['load_address']:08X}\n"
    )
    splat_text = (
        f"name: {basename}\nsha1: {hashlib.sha1(payload.read_bytes()).hexdigest()}\n"
        "options:\n  platform: psx\n  compiler: psyq\n"
        f"  basename: {basename}\n  base_path: {_base_path(Path(splat))}\n"
        f"  target_path: {binary}\n  asm_path: out/splat/emi/{slug}/asm\n"
        f"  src_path: src/emi/{slug}\n  ld_script_path: out/splat/emi/{slug}/linker.ld\n"
        "  symbol_addrs_path:\n"
        "  - config/targets/shared/symbols.txt\n"
        "  - config/sdk/psyq-slus.txt\n"
        f"  - {symbols}\n"
        f"segments:\n- [0x0, bin]\n- [0x{entry['size']:X}]\n"
    )
    return {
        "schema": "harness.emi-bootstrap/v1",
        "entry": entry["id"],
        "target": target,
        "identity": {
            "payload_sha256": entry["sha256"],
            "load_address": entry["load_address"],
            "size": entry["size"],
        },
        "files": [
            {"path": binary, "kind": "payload"},
            {"path": f"{binary}.json", "kind": "text", "content": metadata},
            {"path": manifest, "kind": "text", "content": manifest_text},
            {"path": splat, "kind": "text", "content": splat_text},
            {"path": symbols, "kind": "text", "content": ""},
        ],
    }


def apply_bootstrap(
    root: Path, catalog: dict[str, Any], plan: dict[str, Any]
) -> list[Path]:
    fresh = bootstrap_plan(root, catalog, str(plan.get("entry", "")))
    if fresh != plan:
        raise ValueError("bootstrap plan is stale")
    created: list[Path] = []
    try:
        payload = Path(
            resolve_entry(catalog, fresh["entry"])["payload_path"]
        ).read_bytes()
        for item in fresh["files"]:
            path = _destination(root, item["path"])
            content = payload if item["kind"] == "payload" else item["content"].encode()
            _write_new(path, content)
            created.append(path)
        manifests = load_target_manifests(root)
        if fresh["target"] not in manifests:
            raise ValueError("created target manifest did not load")
        load_map(root / f"config/targets/{fresh['target']}/symbols.txt")
        return created
    except BaseException:
        for path in reversed(created):
            path.unlink(missing_ok=True)
        raise


def materialize_reviewed_targets(*, root: Path, catalog: dict[str, Any]) -> list[Path]:
    """Restore ignored binaries for already-reviewed EMI manifests."""

    verify_companion_relations(root, catalog)
    images: list[Path] = []
    for manifest in sorted(
        load_target_manifests(root).values(), key=lambda item: item.id.value
    ):
        if manifest.kind != "emi":
            continue
        entry = resolve_entry(catalog, manifest.disc_id)
        source = Path(entry["payload_path"])
        if not source.is_file() or file_sha256(source) != entry["sha256"]:
            raise ValueError(f"missing or stale payload for {manifest.disc_id}")
        if manifest.load_address != int(entry["load_address"]):
            raise ValueError(
                f"load address differs from {manifest.disc_id}: "
                f"manifest=0x{manifest.load_address:08X}, "
                f"payload=0x{int(entry['load_address']):08X}"
            )
        image = root / manifest.binary
        if image.is_file() and file_sha256(image) != entry["sha256"]:
            raise ValueError(
                f"normalized binary differs from {manifest.disc_id}: {image}"
            )
        if not image.is_file():
            _write_new(image, source.read_bytes())
        write_json(
            image.with_suffix(".bin.json"),
            {
                "schema": "harness.normalized-emi/v1",
                "source": str(source),
                "source_sha256": entry["sha256"],
                "image": str(image),
                "image_sha256": entry["sha256"],
                "load_address": manifest.load_address,
            },
        )
        images.append(image)
    return images


def _write_new(path: Path, content: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists():
        raise ValueError(f"refusing to overwrite: {path}")
    descriptor, temporary_name = tempfile.mkstemp(
        dir=path.parent, prefix=f".{path.name}."
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(content)
            stream.flush()
            os.fsync(stream.fileno())
        os.link(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def _destination(root: Path, relative: str) -> Path:
    path = (root / relative).resolve()
    if not path.is_relative_to(root.resolve()):
        raise ValueError(f"target path escapes repository: {relative}")
    return path


def _base_path(config_path: Path) -> str:
    return "/".join(".." for _ in config_path.parts[:-1]) or "."


def _eligibility(root: Path, entry: dict[str, Any]) -> list[str]:
    reasons: list[str] = []
    payload = Path(entry["payload_path"])
    if not payload.is_file():
        reasons.append("missing-payload")
    elif file_sha256(payload) != entry["sha256"]:
        reasons.append("payload-sha256-mismatch")
    if entry.get("payload_kind") != "ram":
        reasons.append(f"not-ram:{entry.get('payload_kind', 'unknown')}")
    if entry.get("code_status") == "rejected":
        reasons.append("rejected")
    address, size = int(entry.get("load_address", 0)), int(entry.get("size", 0))
    if not 0x80000000 <= address < 0x80200000 or address + size > 0x80200000:
        reasons.append("invalid-ram-range")
    if size <= 0:
        reasons.append("empty-payload")
    slug = target_slug(entry)
    tracked = [
        root / f"config/targets/emi/{slug}/target.toml",
        root / f"config/targets/emi/{slug}/splat.yaml",
        root / f"config/targets/emi/{slug}/symbols.txt",
        root / f"src/emi/{slug}",
    ]
    if any(path.exists() for path in tracked):
        reasons.append("existing-reviewed-target")
    return reasons
