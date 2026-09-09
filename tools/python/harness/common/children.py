"""Linux subreaper ownership for descendants that leave their process group."""

from __future__ import annotations

import ctypes
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
