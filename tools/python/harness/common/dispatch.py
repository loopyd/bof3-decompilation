"""Dispatch descriptions for the canonical harness command tree."""

from __future__ import annotations

from collections.abc import Mapping
from dataclasses import dataclass
from importlib import import_module

Argv = list[str]


@dataclass(frozen=True)
class Action:
    """One owner entry point reachable from the command tree."""

    help: str
    module: str
    #: ``passthrough`` keeps the action token for the owner's own subparser,
    #: ``label`` drops it, ``fixed`` replaces it with ``name``.
    kind: str = "label"
    entry: str = "main"
    name: str = ""
    example: str = ""

    def run(self, argv: Argv) -> int:
        rest = list(argv)
        if self.kind == "label":
            rest = rest[1:]
        elif self.kind == "fixed":
            rest = [self.name, *rest[1:]]
        entry = getattr(import_module(self.module), self.entry)
        return entry(rest)


@dataclass(frozen=True)
class Domain:
    """A command domain: explicit actions plus an optional owner fallback."""

    help: str
    actions: Mapping[str, Action]
    passthrough: Action | None = None
