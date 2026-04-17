from __future__ import annotations

import sys
from dataclasses import dataclass


@dataclass(frozen=True)
class Rebof3Logger:
    name: str
    quiet: bool = False
    verbose: bool = False

    def _prefix(self) -> str:
        return f"[{self.name}]"

    def info(self, message: str) -> None:
        if not self.quiet:
            print(f"{self._prefix()} {message}")

    def detail(self, message: str) -> None:
        if self.verbose and not self.quiet:
            print(f"{self._prefix()} {message}")

    def debug(self, message: str) -> None:
        """Alias used by task/pipeline code when emitting debug-oriented detail."""

        self.detail(message)

    def summary(self, message: str) -> None:
        if not self.quiet:
            print(message)

    def item(self, message: str) -> None:
        if not self.quiet:
            print(message)

    def error(self, message: str) -> None:
        print(f"{self._prefix()} {message}", file=sys.stderr)

    def child(self, suffix: str) -> "Rebof3Logger":
        """Return a scoped logger that inherits the current verbosity settings."""

        child_name = suffix if not self.name else f"{self.name}.{suffix}"
        return Rebof3Logger(
            name=child_name,
            quiet=self.quiet,
            verbose=self.verbose,
        )


def make_logger(
    name: str, *, quiet: bool = False, verbose: bool = False
) -> Rebof3Logger:
    return Rebof3Logger(name=name, quiet=quiet, verbose=verbose)
