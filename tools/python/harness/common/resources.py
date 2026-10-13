"""Select native build concurrency from available Linux CPU capacity."""

from __future__ import annotations

import math
import os
from pathlib import Path
import time


def _read_cpu_times(allowed: set[int]) -> dict[int, tuple[int, int]]:
    counters = {}
    for line in Path("/proc/stat").read_text().splitlines():
        name, *fields = line.split()
        if not name.startswith("cpu") or not name[3:].isdigit():
            continue
        cpu = int(name[3:])
        if cpu in allowed:
            ticks = [int(value) for value in fields[:8]]
            counters[cpu] = (sum(ticks), ticks[3] + ticks[4])
    return counters


def _read_cpu_capacity(allowed: set[int]) -> float:
    capacity = float(len(allowed))
    root = Path("/sys/fs/cgroup")
    try:
        group = next(
            line[3:]
            for line in Path("/proc/self/cgroup").read_text().splitlines()
            if line.startswith("0::")
        )
    except (OSError, StopIteration):
        return capacity
    directory = root / group.lstrip("/")
    for parent in (directory, *directory.parents):
        if not parent.is_relative_to(root):
            break
        try:
            quota, period = (parent / "cpu.max").read_text().split()
            if quota != "max":
                capacity = min(capacity, int(quota) / int(period))
        except (OSError, ValueError, ZeroDivisionError):
            continue
    return capacity


def select_build_jobs() -> int:
    """Use 75% of a 200 ms idle-CPU sample, capped by affinity and CPU quota."""
    try:
        allowed = os.sched_getaffinity(0)
        capacity = _read_cpu_capacity(allowed)
        before = _read_cpu_times(allowed)
        time.sleep(0.2)
        after = _read_cpu_times(allowed)
    except (OSError, ValueError, IndexError):
        return 1
    idle = 0.0
    for cpu in before.keys() & after.keys():
        total = after[cpu][0] - before[cpu][0]
        if total > 0:
            idle += min(1.0, max(0.0, (after[cpu][1] - before[cpu][1]) / total))
    return max(1, math.floor(min(capacity, idle) * 0.75))
