"""Shared deterministic file and target evidence for agent context prefills."""

from __future__ import annotations

from pathlib import Path
import re
import sqlite3

from ..domain.ids import FunctionId, normalize_target_id
from .base import ContextSection
from .stable import (
    binding_sources as stable_binding_sources,
    load_manifest as load_stable_manifest,
    resolve_function as resolve_stable_function,
)


FULL_PATHS = (
    "SOUL.md",
    "AGENTS.md",
    "docs/agents/CODING_STANDARDS.md",
    ".pi/skills/bof3-re/SKILL.md",
    "docs/agents/memory-api.md",
    "docs/agents/matching.md",
    "docs/agents/matching-playbook.md",
    "docs/agents/project-context.md",
    "docs/agents/plan-authoring.md",
    "docs/agents/lessons.md",
)
IDENTIFIER = re.compile(r"\b(?:D|func)_[0-9A-Fa-f]{8}\b")
_CONTRACT_PATHS = (
    "SOUL.md",
    "AGENTS.md",
    "docs/agents/memory-api.md",
    "docs/agents/matching.md",
    "docs/agents/matching-playbook.md",
    "docs/agents/lessons.md",
)


def contract_sections(root: Path, role: str) -> list[ContextSection]:
    """Return complete tracked contract documents in deterministic order."""

    paths = [*_CONTRACT_PATHS]
    if role == "reverse":
        paths.append("docs/reference/bof3-eu/README.md")
    sections = []
    for relative in paths:
        path = root / relative
        if not path.is_file():
            raise ValueError(f"missing required context: {relative}")
        sections.append(ContextSection(relative, path.read_text(encoding="utf-8")))
    return sections


def _lookup_manifest(root: Path, value: str):
    from ..domain.manifests import load_target_manifests

    target = normalize_target_id(value)
    return load_target_manifests(root).get(target.value)


def _identifier_names(source: Path | None, address: int, excerpt: str) -> set[str]:
    names = set(IDENTIFIER.findall(excerpt)) | {f"func_{address:08X}"}
    if source is not None and source.is_file():
        names.update(IDENTIFIER.findall(source.read_text(encoding="utf-8")))
    return names


def _line_has_name(line: str, names: set[str]) -> bool:
    return any(re.search(rf"\b{re.escape(name)}\b", line) for name in names)


def _stable_binding_excerpt(path: Path, names: set[str]) -> str:
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
    return "".join(line for line in lines if _line_has_name(line, names))


def _stable_target_pairs(root: Path, function: FunctionId):
    resolved = resolve_stable_function(root, function)
    manifest = resolved.manifest
    target = manifest.id
    address = function.address
    manifest_path = root / "config/targets" / target / "target.toml"
    map_path = root / "config/targets" / target / "symbols.txt"
    splat = root / manifest.splat
    source = resolved.source
    map_text = map_excerpt(map_path, address) if map_path.is_file() else ""
    names = _identifier_names(source, address, map_text)
    if resolved.compiled_symbol is not None:
        names.add(resolved.compiled_symbol)
    pairs: list[tuple[Path, str | None]] = [
        (manifest_path, manifest_excerpt(root, manifest_path, source))
    ]
    if map_path.is_file():
        pairs.append((map_path, map_text))
    if splat.is_file():
        lines = splat.read_text(encoding="utf-8").splitlines(keepends=True)
        splat_text = "".join(lines[:16]) + around_any(
            lines, {f"0x{address:x}", f"func_{address:08X}"}
        )
        pairs.append((splat, splat_text))
    headers = [root / path for path in manifest.headers if (root / path).is_file()]
    if source is not None and (local := source.parent / "internal.h").is_file():
        headers.append(local)
    for header in dict.fromkeys(headers):
        excerpt = header_excerpt(header, names)
        if excerpt:
            pairs.append((header, excerpt))
    for binding in stable_binding_sources(root, manifest):
        if binding.is_file() and (excerpt := _stable_binding_excerpt(binding, names)):
            pairs.append((binding, excerpt))
    if source is not None and source.is_file():
        pairs.append((source, source_excerpt(source)))
    return manifest, pairs


def stable_target_sections(root: Path, function: FunctionId) -> list[ContextSection]:
    """Return bounded tracked selector facts; never read generated evidence."""

    _manifest, pairs = _stable_target_pairs(root, function)
    return _sections(root, pairs)


def selector_sections(
    root: Path, function: FunctionId | None, mode: str
) -> list[ContextSection]:
    """Return selector evidence appropriate to the requested context mode."""

    if function is None:
        return []
    renderer = stable_target_sections if mode == "stable" else target_sections
    return renderer(root, function)


def roster_sections(root: Path) -> list[ContextSection]:
    """Return sorted agent and skill summaries."""

    agents: list[str] = []
    for path in sorted((root / ".pi" / "agents").glob("*.md")):
        parts = path.read_text(encoding="utf-8").split("---")
        front = parts[1] if len(parts) > 2 else ""
        fields = dict(
            line.split(": ", 1) for line in front.splitlines() if ": " in line
        )
        agents.append(
            f"{fields.get('name', path.stem)}: {fields.get('description', '')}"
        )
    agents.sort(key=lambda value: value.split(":", 1)[0])
    skills = sorted(
        path.parent.name for path in (root / ".pi/skills").glob("*/SKILL.md")
    )
    return [
        ContextSection("subagent roster (.pi/agents)", "\n".join(agents) + "\n"),
        ContextSection("skills (.pi/skills)", "\n".join(skills) + "\n"),
    ]


def around(lines: list[str], needle: str, radius: int = 5) -> str:
    return around_any(lines, {needle}, radius)


def around_any(lines: list[str], needles: set[str], radius: int = 5) -> str:
    for index, line in enumerate(lines):
        if any(needle.lower() in line.lower() for needle in needles):
            return "".join(lines[max(0, index - radius) : index + radius + 1])
    return ""


def manifest_excerpt(root: Path, path: Path, source: Path | None) -> str:
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
    keep = lines[:11]
    if source is not None:
        source_name = source.relative_to(root).as_posix()
        keep.extend(line for line in lines[11:] if source_name in line)
    return "".join(keep)


def source_excerpt(path: Path, line_limit: int = 180) -> str:
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
    if len(lines) <= line_limit:
        return "".join(lines)
    return "".join(lines[:line_limit]) + "/* context excerpt truncated */\n"


def map_excerpt(path: Path, address: int) -> str:
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
    index = next(
        (
            index
            for index, line in enumerate(lines)
            if (match := re.search(r"=\s*(0x[0-9A-Fa-f]+);", line))
            and int(match.group(1), 0) >= address
        ),
        len(lines),
    )
    return "".join(lines[max(0, index - 3) : index + 3])


def header_excerpt(path: Path, names: set[str]) -> str:
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
    boundary = next(
        (
            index
            for index, line in enumerate(lines)
            if "Absolute-address globals" in line
        ),
        min(len(lines), 120),
    )
    output = lines[:boundary]
    output.extend(line for line in lines[boundary:] if _line_has_name(line, names))
    return "".join(output)


def asm_path(root: Path, target: str, splat: Path, address: int) -> Path:
    from ..domain.sources import CompiledSymbolError, reviewed_function_name

    match = re.search(
        r"^\s*asm_path:\s*(\S+)\s*$",
        splat.read_text(encoding="utf-8"),
        re.MULTILINE,
    )
    directory = root / match.group(1) if match else root / "out/splat" / target / "asm"
    try:
        name = reviewed_function_name(root, target, address)
    except (CompiledSymbolError, ValueError):
        name = None
    if name is not None and (candidate := directory / f"{name}.s").is_file():
        return candidate
    return directory / f"func_{address:08X}.s"


def unlabeled_refs(root: Path, function: FunctionId, manifest) -> str:
    index = root / "out/index/reverse.sqlite"
    binary = root / manifest.binary
    if not index.is_file() or not binary.is_file():
        return ""
    load = manifest.load_address
    end = load + binary.stat().st_size
    try:
        connection = sqlite3.connect(f"file:{index}?mode=ro", uri=True)
        try:
            own = [
                f"0x{row[0]:08X}"
                for row in connection.execute(
                    "SELECT address FROM data_references "
                    "WHERE function_id = ? AND symbol IS NULL ORDER BY address",
                    (function.value,),
                )
                if load <= row[0] < end
            ]
            hot = [
                (row[0], row[1])
                for row in connection.execute(
                    "SELECT address, COUNT(*) FROM data_references "
                    "WHERE target_id = ? AND symbol IS NULL "
                    "GROUP BY address ORDER BY 2 DESC LIMIT 8",
                    (function.target.value,),
                )
                if load <= row[0] < end
            ]
        finally:
            connection.close()
    except sqlite3.DatabaseError:
        return ""
    lines = ["unlabeled data references (label in the target map when proven):"]
    if own:
        lines.append("this function: " + " ".join(own))
    if hot:
        lines.append(
            "target hot gaps: "
            + " ".join(f"0x{address:08X}({refs})" for address, refs in hot)
        )
    return "\n".join(lines)


def target_audit_sections(root: Path, target: str) -> list[ContextSection]:
    manifest = load_stable_manifest(root, target)
    if manifest is None:
        raise ValueError(f"unknown target: {target}")
    target = manifest.id
    manifest_path = root / "config/targets" / target / "target.toml"
    map_path = root / "config/targets" / target / "symbols.txt"
    splat = root / manifest.splat
    pairs: list[tuple[Path, str | None]] = [
        (manifest_path, manifest_excerpt(root, manifest_path, None))
    ]
    if map_path.is_file():
        pairs.append((map_path, source_excerpt(map_path, 120)))
    if splat.is_file():
        pairs.append((splat, source_excerpt(splat, 120)))
    for claimed in manifest.headers:
        path = root / claimed
        if path.is_file():
            pairs.append((path, source_excerpt(path, 120)))
    return _sections(root, pairs)


def target_sections(root: Path, function: FunctionId) -> list[ContextSection]:
    from ..domain.claims import (
        manifest_binding_sources,
        resolve_manifest_source_for_address,
    )

    manifest = _lookup_manifest(root, function.target.value)
    if manifest is None:
        raise ValueError(f"unknown target: {function.target.value}")
    target = manifest.id.value
    address = function.address
    manifest_path = root / "config/targets" / target / "target.toml"
    map_path = root / "config/targets" / target / "symbols.txt"
    splat = root / manifest.splat
    source = resolve_manifest_source_for_address(root, manifest, address)
    assembly = asm_path(root, target, splat, address)
    asm_text = assembly.read_text(encoding="utf-8") if assembly.is_file() else ""
    names = set(IDENTIFIER.findall(asm_text)) | {f"func_{address:08X}"}
    pairs: list[tuple[Path, str | None]] = [(manifest_path, None)]
    if map_path.is_file():
        pairs.append((map_path, map_excerpt(map_path, address)))
    if splat.is_file():
        lines = splat.read_text(encoding="utf-8").splitlines(keepends=True)
        pairs.append(
            (splat, "".join(lines[:16]) + around(lines, f"func_{address:08X}"))
        )
    headers = [root / path for path in manifest.headers if (root / path).is_file()]
    if source is not None and (local := source.parent / "internal.h").is_file():
        headers.append(local)
    source_dir = root / manifest.source_dir
    if (legacy := source_dir / "internal.h").is_file():
        headers.append(legacy)
    if (public := source_dir / "public").is_dir():
        headers.extend(sorted(public.glob("*.h")))
    for header in dict.fromkeys(headers):
        pairs.append((header, header_excerpt(header, names)))
    pairs.extend(
        (binding, None)
        for binding in manifest_binding_sources(root, manifest)
        if binding.is_file()
    )
    if source is not None and source.is_file():
        pairs.append((source, None))
    if assembly.is_file():
        pairs.append((assembly, asm_text))
    sections = [
        ContextSection(
            path.relative_to(root).as_posix(),
            text if text is not None else path.read_text(encoding="utf-8"),
        )
        for path, text in pairs
    ]
    refs = unlabeled_refs(root, function, manifest)
    if refs:
        sections.append(
            ContextSection(f"data-scan: {target}", refs + "\n", leading_newline=False)
        )
    return sections


def _sections(root: Path, pairs: list[tuple[Path, str | None]]) -> list[ContextSection]:
    return [
        ContextSection(
            path.relative_to(root).as_posix(),
            text if text is not None else path.read_text(encoding="utf-8"),
        )
        for path, text in pairs
    ]


__all__ = [
    "FULL_PATHS",
    "around",
    "asm_path",
    "contract_sections",
    "header_excerpt",
    "map_excerpt",
    "roster_sections",
    "selector_sections",
    "stable_target_sections",
    "target_audit_sections",
    "target_sections",
    "unlabeled_refs",
]
