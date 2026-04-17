from __future__ import annotations

import os
import subprocess
from pathlib import Path


def run_command(
    command: list[str],
    *,
    cwd: Path | None = None,
    env: dict[str, str] | None = None,
) -> None:
    full_env = os.environ.copy()
    if env is not None:
        full_env.update(env)
    print("+", " ".join(command))
    result = subprocess.run(command, cwd=cwd, env=full_env, check=False)
    if result.returncode != 0:
        raise RuntimeError(
            f"command failed with exit code {result.returncode}: {' '.join(command)}"
        )


def ensure_parent(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
