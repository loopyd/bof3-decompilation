from __future__ import annotations

from dataclasses import dataclass
from typing import Callable

from .models import SetupContext, SetupOptions, SetupTask

TaskRunner = Callable[[SetupContext], None]
TaskEnabled = Callable[[SetupOptions], bool]


@dataclass(frozen=True)
class SetupTaskSpec:
    task: SetupTask
    runner: TaskRunner
    enabled: TaskEnabled
