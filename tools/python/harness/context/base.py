"""Registration, validation, and rendering for agent context profiles."""

from __future__ import annotations

from collections.abc import Callable, Iterable
from dataclasses import dataclass
from importlib import import_module
from pathlib import Path
from typing import TYPE_CHECKING

from ..domain.ids import FunctionId

if TYPE_CHECKING:
    from .bof3_cleanup import CleanupRequest


_PREFILL_NOTE = (
    "This command is the run's context prefill. Do not rerun it or reread an "
    "emitted path unless a named evidence gap requires newer or unbundled facts. "
    "The working directory is the repository root; use relative commands without cd."
)


@dataclass(frozen=True)
class ContextRequest:
    """One validated context rendering request."""

    root: Path
    role: str
    function: FunctionId | None = None
    target: str | None = None
    mode: str = "stable"
    cleanup: CleanupRequest | None = None


@dataclass(frozen=True)
class ContextSection:
    """One named, ordered block in rendered context."""

    name: str
    text: str
    leading_newline: bool = True

    def render(self) -> str:
        prefix = "\n" if self.leading_newline else ""
        return f"{prefix}===== {self.name} =====\n{self.text}"


ContextHandler = Callable[[ContextRequest], Iterable[ContextSection]]


@dataclass(frozen=True)
class ContextProfile:
    """Immutable metadata and renderer for one concrete agent role."""

    name: str
    paths: tuple[str, ...]
    accepts_selector: bool
    accepts_target: bool
    stable_paths: tuple[str, ...] | None
    byte_limit: int
    section_limit: int
    handler: ContextHandler


_PROFILES: dict[str, ContextProfile] = {}
_MODULES = (
    ("agents", "agents"),
    ("bof3_reverse", "reverse"),
    ("bof3_review", "review"),
    ("bof3_cleanup", "cleanup"),
    ("classifier", "classifier"),
    ("context_builder", "context-builder"),
    ("oracle", "oracle"),
    ("planner", "planner"),
    ("researcher", "researcher"),
    ("reviewer", "reviewer"),
    ("scout", "scout"),
    ("worker", "worker"),
)
_LOADED = False


def _add_profile(profiles: dict[str, ContextProfile], profile: ContextProfile) -> None:
    """Add one profile to an isolated registry."""

    if profile.name in profiles:
        raise ValueError(f"duplicate agent context profile: {profile.name}")
    profiles[profile.name] = profile


def _context_profile(
    name: str,
    *,
    paths: Iterable[str] = (),
    accepts_selector: bool = False,
    accepts_target: bool = False,
    stable_paths: Iterable[str] | None = None,
    byte_limit: int = 13_000,
    section_limit: int = 4,
) -> Callable[[ContextHandler], ContextHandler]:
    """Register one profile without performing repository I/O."""

    def register(handler: ContextHandler) -> ContextHandler:
        _add_profile(
            _PROFILES,
            ContextProfile(
                name=name,
                paths=tuple(dict.fromkeys(paths)),
                accepts_selector=accepts_selector,
                accepts_target=accepts_target,
                stable_paths=(
                    None if stable_paths is None else tuple(dict.fromkeys(stable_paths))
                ),
                byte_limit=byte_limit,
                section_limit=section_limit,
                handler=handler,
            ),
        )
        return handler

    return register


def _load_profiles() -> None:
    global _LOADED
    if _LOADED:
        return
    for module, _name in _MODULES:
        import_module(f"{__package__}.{module}")
    _LOADED = True


def profile_names() -> tuple[str, ...]:
    """Return built-ins in fixed order, independent of legal prior imports."""

    _load_profiles()
    builtins = tuple(name for _module, name in _MODULES)
    known = set(builtins)
    return (*builtins, *(name for name in _PROFILES if name not in known))


def _profile(name: str) -> ContextProfile:
    _load_profiles()
    try:
        return _PROFILES[name]
    except KeyError as error:
        raise ValueError(f"unknown agent context profile: {name}") from error


def _validate(profile: ContextProfile, request: ContextRequest) -> None:
    if request.mode not in {"stable", "compatibility"}:
        raise ValueError(f"unknown agent context mode: {request.mode}")
    if request.cleanup is not None and profile.name != "cleanup":
        raise ValueError("structured cleanup request requires cleanup role")
    if (
        profile.name == "cleanup"
        and request.cleanup is None
        and request.mode != "compatibility"
    ):
        raise ValueError("stable cleanup context requires a structured request")
    # Compatibility retains selectors historically accepted and ignored by
    # workflow roles. Stable prefills reject task evidence a role cannot own.
    if (
        request.mode == "stable"
        and request.function is not None
        and not profile.accepts_selector
    ):
        raise ValueError(f"{profile.name} context does not accept a function selector")
    if request.target is not None and (
        not profile.accepts_target or request.function is not None
    ):
        raise ValueError("--target requires cleanup role and no function selector")


def _required_sections(
    profile: ContextProfile, request: ContextRequest
) -> list[ContextSection]:
    paths = (
        profile.stable_paths
        if request.mode == "stable" and profile.stable_paths is not None
        else profile.paths
    )
    missing = [path for path in paths if not (request.root / path).is_file()]
    if missing:
        raise ValueError(f"missing required context: {', '.join(missing)}")
    return [
        ContextSection(path, (request.root / path).read_text(encoding="utf-8"))
        for path in paths
    ]


def render_context(
    root: Path,
    role: str,
    selector: FunctionId | None = None,
    target: str | None = None,
    mode: str = "stable",
    cleanup: CleanupRequest | None = None,
) -> str:
    """Render one deterministic, read-only role prefill."""

    profile = _profile(role)
    request = ContextRequest(root.resolve(), role, selector, target, mode, cleanup)
    _validate(profile, request)
    sections = _required_sections(profile, request)
    if mode == "stable":
        sections.append(
            ContextSection("context prefill contract", _PREFILL_NOTE + "\n")
        )
    sections.extend(profile.handler(request))
    if not sections:
        output = f"role {role}: no repository context required\n"
    else:
        output = "".join(section.render() for section in sections).lstrip()
    if mode == "stable":
        encoded = len(output.encode())
        if encoded > profile.byte_limit or len(sections) > profile.section_limit:
            raise ValueError(
                f"stable {role} context exceeds bound: "
                f"{encoded}/{profile.byte_limit} bytes, "
                f"{len(sections)}/{profile.section_limit} sections"
            )
    return output
