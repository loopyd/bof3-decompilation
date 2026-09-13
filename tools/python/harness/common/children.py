"""Linux subreaper ownership for descendants that leave their process group."""

from __future__ import annotations

import ctypes
import errno
import os
from pathlib import Path
import signal
import subprocess
import time


def adopt_children() -> None:
    library = ctypes.CDLL(None, use_errno=True)
    if library.prctl(36, 1, 0, 0, 0) != 0:
        raise OSError(ctypes.get_errno(), "cannot establish child subreaper")
    child_list = Path(f"/proc/self/task/{os.getpid()}/children")
    if child_list.read_text().strip():
        raise RuntimeError("process supervisor started with unexpected children")


def open_child_exit(process_id: int) -> int | None:
    """Return an exit-readiness descriptor, retaining polling on unsupported hosts."""
    library = ctypes.CDLL(None, use_errno=True)
    try:
        open_descriptor = library.pidfd_open
    except AttributeError:
        return None
    open_descriptor.argtypes = (ctypes.c_int, ctypes.c_uint)
    open_descriptor.restype = ctypes.c_int
    descriptor = open_descriptor(process_id, 0)
    if descriptor >= 0:
        return descriptor
    failure = ctypes.get_errno()
    if failure in {errno.ENOSYS, errno.EOPNOTSUPP, errno.EPERM, errno.EACCES}:
        return None
    raise OSError(failure, "cannot observe child exit")


def reap_children(child: subprocess.Popen) -> None:
    child_list = Path(f"/proc/self/task/{os.getpid()}/children")
    while True:
        children = [int(value) for value in child_list.read_text().split()]
        for process_id in children:
            try:
                os.kill(process_id, signal.SIGKILL)
            except ProcessLookupError:
                pass
        child.poll()
        while True:
            try:
                process_id, status = os.waitpid(-1, os.WNOHANG)
            except ChildProcessError:
                break
            if process_id == 0:
                break
            if process_id == child.pid:
                child.returncode = os.waitstatus_to_exitcode(status)
        if not child_list.read_text().strip():
            child.wait()
            return
        time.sleep(0.01)
