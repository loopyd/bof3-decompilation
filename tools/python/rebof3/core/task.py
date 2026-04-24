from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from typing import Any

TaskRunner = Callable[[Any], Any | None]


@dataclass(frozen=True)
class Task:
    name: str
    description: str
    runner: TaskRunner

    def run(self, context: Any) -> Any:
        result = self.runner(context)
        return context if result is None else result
