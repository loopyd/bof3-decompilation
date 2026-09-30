"""Execution helper for repository-owned native tool adapters."""

from __future__ import annotations

import errno
import os
import sys
from collections.abc import Sequence
from typing import NoReturn


def exec_tool(executable: str, argv: Sequence[str]) -> NoReturn:
    """Replace this process with ``executable`` and never return.

    Mirrors POSIX ``exec``: a PATH search applies to names without a slash, a
    missing executable exits 127, and an existing-but-not-executable target
    exits 126.  The shell convention is preserved so migrated native commands
    keep the retired wrappers' launch-failure statuses.
    """

    try:
        os.execvp(executable, [executable, *argv])
    except OSError as error:
        status = 127 if error.errno == errno.ENOENT else 126
        print(f"{executable}: {error.strerror}", file=sys.stderr)
        raise SystemExit(status) from error
