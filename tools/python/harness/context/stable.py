"""Bootstrap-safe stable selector resolution using only the Python standard library."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re
import tomllib

from ..domain.ids import FunctionId, normalize_target_id


@dataclass(frozen=True)
class StableManifest:
    """Manifest fields needed by stable context rendering."""

    id: str
    source_dir: str
    binary: str
    splat: str
    load_address: int
    sources: tuple[str, ...]
    support_sources: tuple[str, ...]
    headers: tuple[str, ...]
    psyq_source: str


def load_manifest(root: Path, value: str) -> StableManifest | None:
    """Load one canonical target manifest without importing harness registries."""

    target = normalize_target_id(value).value
    path = root / "config/targets" / target / "target.toml"
    if not path.is_file():
        return None
    with path.open("rb") as stream:
        raw = tomllib.load(stream)
    if normalize_target_id(str(raw["id"])).value != target:
        raise ValueError(f"target manifest identity mismatch: {target}")
    return StableManifest(
        id=target,
        source_dir=str(raw["source_dir"]),
        binary=str(raw["binary"]),
        splat=str(raw["splat"]),
        load_address=int(raw["load_address"]),
        sources=tuple(map(str, raw.get("sources", ()))),
        support_sources=tuple(map(str, raw.get("support_sources", ()))),
        headers=tuple(map(str, raw.get("headers", ()))),
        psyq_source=str(raw.get("psyq_source", "")),
    )


def resolve_source(root: Path, manifest: StableManifest, address: int) -> Path | None:
    """Resolve a claimed C source by its function-level ``@source`` tag."""

    marker = re.compile(rf"@source\s+0x0*{address:X}\b", re.IGNORECASE)
    for claimed in manifest.sources:
        path = root / claimed
        if (
            path.suffix == ".c"
            and path.is_file()
            and marker.search(path.read_text(encoding="utf-8"))
        ):
            return path
    return None


def compiled_symbol(
    root: Path, manifest: StableManifest, source: Path | None, address: int
) -> str | None:
    """Return the map symbol when tracked Splat metadata agrees with it."""

    map_path = root / "config/targets" / manifest.id / "symbols.txt"
    splat = root / manifest.splat
    if not map_path.is_file() or not splat.is_file():
        return None
    pattern = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+);", re.MULTILINE)
    map_text = map_path.read_text(encoding="utf-8")
    symbol = next(
        (
            match.group(1)
            for match in pattern.finditer(map_text)
            if int(match.group(2), 0) == address
        ),
        None,
    )
    if symbol is None:
        return None
    splat_text = splat.read_text(encoding="utf-8")
    if source is None:
        agrees = re.search(rf"\b{re.escape(symbol)}\b", splat_text) is not None
    else:
        claimed = source.relative_to(root).as_posix()
        agrees = any(
            re.search(
                rf"\b{re.escape(symbol)}\b",
                splat_text[max(0, match.start() - 300) : match.end()],
            )
            for match in re.finditer(re.escape(claimed), splat_text)
        )
    return symbol if agrees else None


def binding_sources(root: Path, manifest: StableManifest) -> list[Path]:
    """Return target-local hand-maintained binding candidates."""

    candidates = [
        root / claimed
        for claimed in manifest.support_sources
        if Path(claimed).suffix == ".c" and claimed != manifest.psyq_source
    ]
    symbols = sorted(path for path in candidates if path.stem == "symbols")
    return symbols or sorted(candidates)


@dataclass(frozen=True)
class StableFunction:
    manifest: StableManifest
    source: Path | None
    compiled_symbol: str | None


def resolve_function(root: Path, function: FunctionId) -> StableFunction:
    """Resolve stable selector facts without registry, layout, or YAML imports."""

    manifest = load_manifest(root, function.target.value)
    if manifest is None:
        raise ValueError(f"unknown target: {function.target.value!r}")
    source = resolve_source(root, manifest, function.address)
    return StableFunction(
        manifest, source, compiled_symbol(root, manifest, source, function.address)
    )
