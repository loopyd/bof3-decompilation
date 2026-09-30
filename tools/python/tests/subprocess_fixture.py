"""Shared subprocess support for tests that invoke repository commands.

One owner for the two things every such test needs: a captured, non-raising
command run rooted at the repository, and an ambient environment with the
bootstrap's private overrides removed.  Tests import these directly; the
helpers hold no state, so sharing them cannot leak between tests.
"""

from __future__ import annotations

import os
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]

#: Private ``bin/python-env`` overrides that must never reach a called command.
_PRIVATE_KEYS = frozenset(
    {"PSX_PYTHON", "PYTHON_ENV_EXIT", "PYTHON_ENV_HINT", "PYTHON_ENV_PYTHON"}
)


def clean_env() -> dict[str, str]:
    """Ambient environment without the bootstrap's private overrides."""

    return {key: value for key, value in os.environ.items() if key not in _PRIVATE_KEYS}


def run_command(
    *args: str,
    env: dict[str, str] | None = None,
    cwd: Path = REPO_ROOT,
) -> subprocess.CompletedProcess[str]:
    """Run one command from the repository root and capture its streams."""

    return subprocess.run(
        args,
        cwd=cwd,
        capture_output=True,
        text=True,
        env=env,
        check=False,
    )
