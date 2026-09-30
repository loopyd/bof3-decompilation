"""``bin/harness agent context``: isolated bounded context emission.

The stable contract renders context in a site-isolated interpreter (``-S``),
while the bounded legacy-compatible mode needs the ambient interpreter.  The
choice is a trust boundary, so this owner re-execs the rendering process with
the matching isolation instead of forwarding to the shared dispatcher.
"""

from __future__ import annotations

import sys

from harness.common.native import exec_tool

_ISOLATED = "-S"
_COMPATIBILITY_TOKENS = {"--mode=compatibility", "compatibility"}


def main(argv: list[str] | None = None) -> int:
    args = list(argv or [])
    compatibility = any(token in _COMPATIBILITY_TOKENS for token in args)
    command: list[str] = []
    if not compatibility:
        command.append(_ISOLATED)
    command += ["-m", "harness.commands.agent_context", *args]
    exec_tool(sys.executable, command)
