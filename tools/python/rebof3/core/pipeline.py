from __future__ import annotations

from collections.abc import Iterable
from dataclasses import dataclass
from typing import Any

from .task import Task


@dataclass(frozen=True)
class Pipeline:
    name: str
    description: str
    tasks: tuple[Task, ...]

    def __init__(
        self,
        name: str,
        description: str,
        tasks: Iterable[Task],
    ) -> None:
        object.__setattr__(self, "name", name)
        object.__setattr__(self, "description", description)
        object.__setattr__(self, "tasks", tuple(tasks))

    def plan(self) -> list[Task]:
        return list(self.tasks)

    def run(self, context: Any | None = None) -> Any:
        current = context
        for task in self.plan():
            current = task.run(current)
        return current
