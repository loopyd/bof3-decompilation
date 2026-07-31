"""In-memory, target-qualified Rizin replay composition."""

from __future__ import annotations

import hashlib
import json
from dataclasses import dataclass
from pathlib import Path

from .analyzer import EngineIdentity, build_snapshot, find_engine
from .canonical import Symbol, load_target_symbols
from .domain import lookup_target_manifest
from .layout import parse_splat_layout
from .snapshot import SNAPSHOT_SCHEMA, snapshot_path, write_snapshot


@dataclass(frozen=True)
class RizinTarget:
    target: str
    binary: Path
    load_address: int
    source_dir: Path
    snapshot: Path
    reviewed_addresses: frozenset[int]
    replay: str
    replay_sha256: str


def _baseline(symbols: list[Symbol], roots: frozenset[int]) -> str:
    lines = [
        "e asm.arch=mips",
        "e asm.bits=32",
        "e cfg.bigendian=false",
        "e scr.color=0",
        "fs bof3",
    ]
    for address in sorted(roots):
        lines.append(f"af @ 0x{address:08X}")
    for symbol in symbols:
        name = symbol.canonical_name
        if name.startswith("func_"):
            lines.extend(
                (f"af @ 0x{symbol.address:08X}", f"afn {name} 0x{symbol.address:08X}")
            )
        else:
            lines.append(f"f {name} 4 @ 0x{symbol.address:08X}")
    return "\n".join(lines) + "\n"


def _reviewed_overlay(path: Path) -> str:
    if not path.is_file():
        return ""
    return path.read_text(encoding="utf-8")


def replay_commands(replay: str) -> list[str]:
    return [
        line.strip()
        for line in replay.splitlines()
        if line.strip() and not line.lstrip().startswith("#")
    ]


def prepare_target(root: Path, target_id: str) -> RizinTarget:
    """Compose a target recipe without writing generated project files."""

    manifest = lookup_target_manifest(root, target_id)
    if manifest is None:
        raise ValueError(f"unknown target: {target_id}")
    binary = root / manifest.binary
    if not binary.is_file():
        raise FileNotFoundError(f"target binary not found: {manifest.binary}")
    splat = root / manifest.splat
    layout = parse_splat_layout(splat, manifest.load_address)
    roots = frozenset(layout.reviewed_function_addresses)
    binary_end = manifest.load_address + binary.stat().st_size
    invalid_roots = sorted(
        address
        for address in roots
        if address % 4 or not manifest.load_address <= address < binary_end
    )
    if invalid_roots:
        rendered = ", ".join(f"0x{address:08X}" for address in invalid_roots)
        raise ValueError(f"reviewed function roots outside target image: {rendered}")
    overlay = root / "config" / "targets" / manifest.id.value / "reviewed.rz"
    replay = _baseline(
        load_target_symbols(root, manifest.id.value), roots
    ) + _reviewed_overlay(overlay)
    return RizinTarget(
        target=manifest.id.value,
        binary=binary,
        load_address=manifest.load_address,
        source_dir=root / manifest.source_dir,
        snapshot=snapshot_path(root, manifest.id.value),
        reviewed_addresses=roots,
        replay=replay,
        replay_sha256=hashlib.sha256(replay.encode()).hexdigest(),
    )


def analyze_project(root: Path, target_id: str, *, timeout: int = 120) -> RizinTarget:
    target = prepare_target(root, target_id)
    engine = find_engine("rizin", root=root)
    snapshot = build_snapshot(
        engine,
        target.binary,
        target.load_address,
        target.target,
        reviewed_addresses=set(target.reviewed_addresses),
        replay_commands=replay_commands(target.replay),
        replay_sha256=target.replay_sha256,
        source_dir=target.source_dir,
        timeout=timeout,
    )
    write_snapshot(snapshot, target.snapshot)
    return target


def status(root: Path, target_id: str) -> dict[str, object]:
    target = prepare_target(root, target_id)
    binary_sha256 = hashlib.sha256(target.binary.read_bytes()).hexdigest()
    snapshot_exists = target.snapshot.is_file()
    fresh = False
    if snapshot_exists:
        try:
            payload = json.loads(target.snapshot.read_text(encoding="utf-8"))
            fresh = (
                payload.get("schema") == SNAPSHOT_SCHEMA
                and payload.get("target") == target.target
                and payload.get("engine", {}).get("name") == "rizin"
                and payload.get("inputs", {}).get("binary_sha256") == binary_sha256
                and payload.get("inputs", {}).get("replay_sha256")
                == target.replay_sha256
            )
        except (OSError, ValueError, json.JSONDecodeError):
            fresh = False
    return {
        "target": target.target,
        "snapshot": str(target.snapshot.relative_to(root)),
        "snapshot_exists": snapshot_exists,
        "fresh": fresh,
        "binary_sha256": binary_sha256,
        "replay_sha256": target.replay_sha256,
    }


def rizin_argv(target: RizinTarget, engine: EngineIdentity) -> list[str]:
    argv = [
        str(engine.executable),
        "-N",
        "-n",
        "-a",
        "mips",
        "-b",
        "32",
        "-e",
        "cfg.bigendian=false",
        "-m",
        f"0x{target.load_address:08X}",
    ]
    for command in replay_commands(target.replay):
        argv.extend(("-c", command))
    argv.append(str(target.binary))
    return argv


__all__ = [
    "RizinTarget",
    "analyze_project",
    "prepare_target",
    "replay_commands",
    "rizin_argv",
    "status",
]
