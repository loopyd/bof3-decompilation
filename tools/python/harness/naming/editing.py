"""Shared locking for naming report-set mutations."""

from __future__ import annotations

import fcntl
import os
from contextlib import contextmanager
from contextvars import ContextVar
from functools import wraps
from pathlib import Path
from typing import Iterator

from harness.common.deadlines import check_deadline
from harness.naming.history import require_mutable_report

_LOCKS: ContextVar[dict | None] = ContextVar("naming_report_locks", default=None)


def report_set_lock_path(path: Path, *, report: bool = False) -> Path:
    """Return the stable sibling lock for a report set or one report in it."""

    report_set = path.parent if report else path
    return report_set.parent / f".{report_set.name}.lock"


@contextmanager
def report_mutation(path: Path, *, report: bool = False) -> Iterator[None]:
    """Serialize a report-set mutation without placing the lock inside it."""

    check_deadline()
    lock_path = report_set_lock_path(path, report=report)
    retained = _LOCKS.get() or {}
    key = str(lock_path.absolute())
    active = retained.get(key)
    if active is not None and active[0] == os.getpid():
        descriptor = os.fstat(active[1])
        named = lock_path.stat()
        if (descriptor.st_dev, descriptor.st_ino) != (named.st_dev, named.st_ino):
            raise ValueError("report lock identity changed during nested mutation")
        if report:
            require_mutable_report(path)
        yield
        return
    lock_path.parent.mkdir(parents=True, exist_ok=True)
    with lock_path.open("a+b") as lock:
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError as error:
            raise ValueError("naming report set is busy; mutation refused") from error
        check_deadline()
        token = _LOCKS.set({**retained, key: (os.getpid(), lock.fileno())})
        try:
            if report:
                require_mutable_report(path)
            yield
        finally:
            _LOCKS.reset(token)


def guard_report_operation(operation):
    """Exclude finalization while an evidence runner owns its report generation."""

    @wraps(operation)
    def run_guarded(root, target, report, *arguments, **keywords):
        from harness.naming.proposal import canonical_report_path

        path = canonical_report_path(root, report)
        with report_mutation(path, report=True):
            return operation(root, target, path, *arguments, **keywords)

    return run_guarded
