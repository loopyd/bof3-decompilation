from __future__ import annotations

from .models import SetupOptions, SetupTask
from .tasks import iter_setup_tasks


def plan_setup_tasks(options: SetupOptions) -> list[SetupTask]:
    return iter_setup_tasks(options)
