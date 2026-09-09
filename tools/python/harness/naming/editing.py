"""Shared locking for naming report-set mutations."""

from __future__ import annotations

import fcntl
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator


def report_set_lock_path(path: Path, *, report: bool = False) -> Path:
    """Return the stable sibling lock for a report set or one report in it."""

    report_set = path.parent if report else path
    return report_set.parent / f".{report_set.name}.lock"


@contextmanager
def report_mutation(path: Path, *, report: bool = False) -> Iterator[None]:
    """Serialize a report-set mutation without placing the lock inside it."""

    lock_path = report_set_lock_path(path, report=report)
    lock_path.parent.mkdir(parents=True, exist_ok=True)
    with lock_path.open("a+b") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        yield
