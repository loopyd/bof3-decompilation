"""Acquire serial evidence clients and attempt every cleanup without hiding uncertainty."""

from __future__ import annotations

import sys
from collections.abc import Iterator
from contextlib import contextmanager
from pathlib import Path

from harness.common.process import ProcessCleanupError
from harness.naming.client import IndexWorker
from harness.naming.native import NativeByteOps
from harness.naming.semantics import RizinSession


@contextmanager
def open_session(
    root: Path,
    target: str,
    report: dict,
    *,
    rizin_executable: str | Path | None,
) -> Iterator[tuple[IndexWorker, RizinSession, NativeByteOps]]:
    worker = rizin = None
    try:
        worker = IndexWorker(
            root,
            target,
            report,
        )
        rizin = RizinSession(
            root,
            target,
            executable=rizin_executable,
        )
        yield worker, rizin, NativeByteOps(root)
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
