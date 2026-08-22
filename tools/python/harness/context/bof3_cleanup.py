"""Parse, route, and render one canonical BOF3 cleanup request."""

from __future__ import annotations

from collections.abc import Callable, Sequence
from dataclasses import dataclass, replace
import json
from pathlib import Path, PurePosixPath
import re

from ..domain.ids import FunctionId, normalize_target_id, parse_function_id
from ..domain.manifests import load_target_manifests
from .base import ContextRequest, ContextSection, _context_profile
from .common import FULL_PATHS, selector_sections, target_audit_sections


_REPOSITORY_ROOT = Path(__file__).resolve().parents[4]
_ROOT_DOCUMENTATION = {"README.md", "AGENTS.md"}
_PREPARED_ROW_KINDS = {"function", "data"}
_IDENTIFIER = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
_CLASS = re.compile(r"^[a-z][a-z0-9_]*$")
_ROW = re.compile(
    r"^(?:(?P<target>(?:exe|emi)/[A-Za-z0-9_./-]+)@)?"
    r"(?P<kind>[a-z][a-z0-9_-]*):(?P<name>[A-Za-z_][A-Za-z0-9_]*)$"
)


_SKILL_REFERENCES = {
    "bof3-identity-maintenance": {
        "identity": ("references/IDENTITY_TRANSACTIONS.md",),
        "retained": (
            "references/IDENTITY_TRANSACTIONS.md",
            "references/BYTE_SAFE_COSMETICS.md",
        ),
        "relocation": ("references/SOURCE_RELOCATION.md",),
    },
    "repo-documentation-repair": {
        "docs": ("references/DOCUMENTATION_REPAIR.md",),
    },
    "bof3-naming-evidence": {
        "audit": ("references/NAMING_AUDIT_V3.md",),
    },
}
_ROUTE = {
    "symbol": ("bof3-identity-maintenance", "identity"),
    "type": ("bof3-identity-maintenance", "identity"),
    "repair": ("bof3-identity-maintenance", "identity"),
    "retained-lift": ("bof3-identity-maintenance", "retained"),
    "relocate-batch": ("bof3-identity-maintenance", "relocation"),
    "docs": ("repo-documentation-repair", "docs"),
    "audit-target": ("bof3-naming-evidence", "audit"),
}


@dataclass(frozen=True)
class SelectedSkill:
    """One fail-closed skill route and its direct route references."""

    name: str
    body: str
    direct_references: tuple[str, ...]


@dataclass(frozen=True)
class CleanupRequest:
    """One normalized cleanup transaction produced only by this parser."""

    mode: str
    target: str | None
    selector: FunctionId | None
    state: str | None
    rows: tuple[str, ...]
    arguments: tuple[str, ...]
    selected_skill: SelectedSkill
    warning: str | None = None

    def as_dict(self, *, loaded_bytes: int | None = None) -> dict[str, object]:
        """Return the stable structured request rendered for the agent."""

        selected: dict[str, object] = {
            "name": self.selected_skill.name,
            "body": self.selected_skill.body,
            "direct_references": list(self.selected_skill.direct_references),
        }
        if loaded_bytes is not None:
            selected["loaded_bytes"] = loaded_bytes
        return {
            "mode": self.mode,
            "target": self.target,
            "selector": str(self.selector) if self.selector else None,
            "state": self.state,
            "rows": list(self.rows),
            "arguments": list(self.arguments),
            "selected_skill": selected,
            "warning": self.warning,
        }


def _selected_skill(mode: str) -> SelectedSkill:
    name, operation = _ROUTE[mode]
    return SelectedSkill(
        name,
        f".pi/skills/{name}/SKILL.md",
        tuple(
            f".pi/skills/{name}/{path}" for path in _SKILL_REFERENCES[name][operation]
        ),
    )


def _known_target(root: Path, value: str) -> str:
    try:
        target = str(normalize_target_id(value))
        manifest = load_target_manifests(root).get(target)
    except (ValueError, TypeError) as error:
        raise ValueError(f"invalid cleanup target: {value}") from error
    if manifest is None:
        raise ValueError(f"unknown cleanup target: {value}")
    return target


def _selector(value: str, target: str) -> FunctionId:
    try:
        selector = parse_function_id(value)
    except ValueError as error:
        raise ValueError(
            "retained-lift requires one target-qualified selector"
        ) from error
    if str(selector.target) != target:
        raise ValueError("cleanup selector target does not match TARGET")
    return selector


def _rows(root: Path, target: str, values: Sequence[str]) -> tuple[str, ...]:
    rows = tuple(values)
    if len(rows) != len(set(rows)):
        raise ValueError("prepared row IDs must be unique")
    if rows != tuple(sorted(rows)):
        raise ValueError("prepared row IDs must use canonical sorted order")
    for value in rows:
        match = _ROW.fullmatch(value)
        if match is None:
            raise ValueError("prepared row IDs must use [TARGET@]KIND:NAME")
        if match.group("kind") not in _PREPARED_ROW_KINDS:
            raise ValueError("prepared row kind must be function or data")
        qualified = match.group("target")
        if qualified is not None and _known_target(root, qualified) != target:
            raise ValueError("prepared row target does not match TARGET")
    return rows


def _has_symlink_component(root: Path, relative: PurePosixPath) -> bool:
    path = root
    for part in relative.parts:
        path /= part
        if path.is_symlink():
            return True
    return False


def _docs_paths(root: Path, values: Sequence[str]) -> tuple[str, ...]:
    if not values:
        raise ValueError("docs requires at least one documentation path")
    if any("\\" in value for value in values):
        raise ValueError(
            "documentation cleanup paths must use canonical forward slashes"
        )
    paths = tuple(values)
    for value in paths:
        relative = PurePosixPath(value)
        candidate = root.joinpath(*relative.parts)
        allowed = value in _ROOT_DOCUMENTATION or (
            len(relative.parts) >= 2
            and relative.parts[0] == "docs"
            and relative.suffix.lower() == ".md"
        )
        if (
            relative.is_absolute()
            or not allowed
            or any(part in {"", ".", ".."} for part in relative.parts)
            or _has_symlink_component(root, relative)
            or not candidate.is_file()
            or not candidate.resolve().is_relative_to(root.resolve())
        ):
            raise ValueError(
                "documentation cleanup paths must be existing regular repository Markdown"
            )
    return paths


def _identifier(value: str, label: str) -> str:
    if not _IDENTIFIER.fullmatch(value):
        raise ValueError(f"{label} must be a nonempty C identifier")
    return value


def _repository_class(root: Path, value: str) -> str:
    if not _CLASS.fullmatch(value) or not (root / "src/bof3" / value).is_dir():
        raise ValueError("relocate-batch CLASS must be one existing src/bof3 class")
    return value


def parse_cleanup_request(
    arguments: Sequence[str],
    *,
    parent_compatibility: bool = False,
    root: Path = _REPOSITORY_ROOT,
) -> CleanupRequest:
    """Parse one of seven canonical forms, plus bounded parent old-audit input."""

    tokens = tuple(arguments)
    if not tokens:
        raise ValueError("cleanup requires one canonical request")
    mode = tokens[0]
    warning = None
    if mode == "audit":
        if not parent_compatibility:
            raise ValueError("old audit form is parent-only; use docs or audit-target")
        try:
            paths = _docs_paths(root, tokens[1:])
        except ValueError as error:
            raise ValueError(
                "old audit accepts documentation paths only; use audit-target TARGET"
            ) from error
        mode = "docs"
        tokens = (mode, *paths)
        warning = "old parent audit form normalized to docs; compatibility is temporary"
    if mode not in _ROUTE:
        raise ValueError(f"unknown cleanup mode: {mode}")

    target: str | None = None
    selector: FunctionId | None = None
    state: str | None = None
    rows: tuple[str, ...] = ()
    values: tuple[str, ...]
    if mode in {"symbol", "type"}:
        if len(tokens) != 5 or tokens[3] != "->":
            raise ValueError(f"{mode} requires TARGET OLD -> NEW")
        target = _known_target(root, tokens[1])
        values = (
            _identifier(tokens[2], f"{mode} OLD"),
            _identifier(tokens[4], f"{mode} NEW"),
        )
    elif mode == "repair":
        if len(tokens) < 2:
            raise ValueError("repair requires TARGET [ROW...]")
        target = _known_target(root, tokens[1])
        rows = _rows(root, target, tokens[2:])
        values = rows
    elif mode == "retained-lift":
        if len(tokens) < 4:
            raise ValueError(
                "retained-lift requires TARGET SELECTOR exact|improved-partial [ROW...]"
            )
        target = _known_target(root, tokens[1])
        selector = _selector(tokens[2], target)
        state = tokens[3]
        if state not in {"exact", "improved-partial"}:
            raise ValueError("retained-lift state must be exact or improved-partial")
        rows = _rows(root, target, tokens[4:])
        values = rows
    elif mode == "relocate-batch":
        if len(tokens) < 4:
            raise ValueError("relocate-batch requires TARGET CLASS SELECTOR...")
        target = _known_target(root, tokens[1])
        class_name = _repository_class(root, tokens[2])
        selectors = tuple(_selector(value, target) for value in tokens[3:])
        values = (class_name, *(str(value) for value in selectors))
    elif mode == "docs":
        values = _docs_paths(root, tokens[1:])
    else:
        if len(tokens) != 2:
            raise ValueError("audit-target requires exactly one TARGET")
        target = _known_target(root, tokens[1])
        values = ()

    return CleanupRequest(
        mode=mode,
        target=target,
        selector=selector,
        state=state,
        rows=rows,
        arguments=values,
        selected_skill=_selected_skill(mode),
        warning=warning,
    )


def _canonical_tokens(cleanup: CleanupRequest) -> tuple[str, ...]:
    if cleanup.mode in {"symbol", "type"} and len(cleanup.arguments) == 2:
        return (
            cleanup.mode,
            cleanup.target or "",
            cleanup.arguments[0],
            "->",
            cleanup.arguments[1],
        )
    if cleanup.mode == "repair":
        return (cleanup.mode, cleanup.target or "", *cleanup.rows)
    if cleanup.mode == "retained-lift":
        return (
            cleanup.mode,
            cleanup.target or "",
            str(cleanup.selector) if cleanup.selector else "",
            cleanup.state or "",
            *cleanup.rows,
        )
    if cleanup.mode == "relocate-batch":
        return (cleanup.mode, cleanup.target or "", *cleanup.arguments)
    if cleanup.mode == "docs":
        return (cleanup.mode, *cleanup.arguments)
    return (cleanup.mode, cleanup.target or "")


def _validate_frozen_request(root: Path, cleanup: CleanupRequest) -> None:
    expected = parse_cleanup_request(_canonical_tokens(cleanup), root=root)
    normalized_warning = (
        "old parent audit form normalized to docs; compatibility is temporary"
        if cleanup.mode == "docs"
        else None
    )
    if cleanup.warning not in {None, normalized_warning}:
        raise ValueError("cleanup request warning is not canonical")
    if cleanup != replace(expected, warning=cleanup.warning):
        raise ValueError("cleanup request including selected_skill is not canonical")


def cleanup_sections(
    root: Path,
    cleanup: CleanupRequest,
    read_bytes: Callable[[Path], bytes] = Path.read_bytes,
) -> list[ContextSection]:
    """Load exactly one selected body and its route-owned direct references."""

    _validate_frozen_request(root, cleanup)
    selected = cleanup.selected_skill
    if cleanup.mode not in _ROUTE or selected != _selected_skill(cleanup.mode):
        raise ValueError("cleanup selected_skill does not match its canonical route")
    paths = (selected.body, *selected.direct_references)
    missing = [path for path in paths if not (root / path).is_file()]
    if missing:
        raise ValueError(f"missing cleanup route context: {', '.join(missing)}")
    loaded = [(path, read_bytes(root / path)) for path in paths]
    sections = [
        ContextSection(
            "cleanup request",
            json.dumps(
                cleanup.as_dict(loaded_bytes=sum(len(data) for _path, data in loaded)),
                indent=2,
                sort_keys=True,
            )
            + "\n",
        )
    ]
    sections.extend(ContextSection(path, data.decode("utf-8")) for path, data in loaded)
    return sections


@_context_profile(
    "cleanup",
    paths=FULL_PATHS,
    accepts_selector=True,
    accepts_target=True,
    stable_paths=(),
    byte_limit=24_000,
    section_limit=6,
)
def cleanup_context(request: ContextRequest) -> list[ContextSection]:
    if request.cleanup is not None:
        return cleanup_sections(request.root, request.cleanup)
    if request.mode != "compatibility":
        raise ValueError("cleanup context requires a structured cleanup request")
    if request.target is not None:
        return target_audit_sections(request.root, request.target)
    return selector_sections(request.root, request.function, request.mode)
