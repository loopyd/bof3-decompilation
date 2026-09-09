"""Acquire serial evidence clients and attempt every cleanup without hiding uncertainty."""

from __future__ import annotations

import sys
import subprocess
import time
from collections.abc import Iterator
from contextlib import contextmanager
from pathlib import Path

from harness.common.process import ProcessCleanupError
from harness.common.deadlines import check_deadline
from harness.naming.client import IndexWorker
from harness.naming.native import NativeByteOps
from harness.naming.semantics import RizinSession


@contextmanager
def open_session(
    root: Path,
    target: str,
    report: dict,
    deadline: int,
    shard_end: float,
    *,
    work_deadline: float | None,
    rizin_executable: str | Path | None,
) -> Iterator[tuple[IndexWorker, RizinSession, NativeByteOps]]:
    options = (
        {"work_deadline": min(work_deadline, shard_end)}
        if work_deadline is not None
        else {}
    )
    worker = rizin = None
    try:
        worker = IndexWorker(
            root,
            target,
            report,
            min(deadline, max(0, shard_end - time.monotonic())),
            **options,
        )
        rizin = RizinSession(
            root,
            target,
            command_timeout=deadline,
            executable=rizin_executable,
            **options,
        )
        yield worker, rizin, NativeByteOps(root, deadline, **options)
    except (TimeoutError, subprocess.TimeoutExpired):
        check_deadline()
        raise
    finally:
        pending = sys.exc_info()[1]
        failure = pending if isinstance(pending, ProcessCleanupError) else None
        for client in (rizin, worker):
            if client is not None:
                try:
                    client.close()
                except BaseException as error:
                    if failure is None or isinstance(error, ProcessCleanupError):
                        failure = error
        if failure is not None:
            raise failure
