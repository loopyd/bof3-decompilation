"""Pinned parent-only lift gates in a legacy-compiler-compatible namespace sandbox."""

from __future__ import annotations

from pathlib import Path
import shutil
import os
import sys

from harness.common.inputs import file_state, input_state
from harness.common.digests import digest
from harness.common.git import git_index_backup
from harness.io import repo_layout
from harness.toolchain.splat import SplatToolchain
from harness.decomp.scope import verify_policy


def capture_context(root: Path, target: str, paths: list[str]) -> dict:
    context = _capture_inputs(root, target, paths)
    executable = shutil.which("bwrap")
    if executable is None:
        raise ValueError("native lift gates require installed bubblewrap")
    path = Path(executable).resolve(strict=True)
    context["tools"]["bwrap"] = {"path": str(path), "state": file_state(path)}
    return context


def build_gate_command(
    root: Path,
    policy: dict,
    expected_digest: str,
    executable: dict,
    command: list[str],
) -> list[str]:
    verify_policy(root, policy, expected_digest, before_dispatch=False)
    if file_state(Path(executable["path"])) != executable["state"]:
        raise ValueError("native gate sandbox executable drifted")
    arguments = [
        executable["path"],
        "--unshare-all",
        "--die-with-parent",
        "--new-session",
        "--cap-drop",
        "ALL",
        "--ro-bind",
        "/",
        "/",
        "--proc",
        "/proc",
        "--dev",
        "/dev",
        "--tmpfs",
        "/tmp",
        "--tmpfs",
        "/run",
        "--ro-bind",
        str(root),
        str(root),
        "--chdir",
        str(root),
        "--setenv",
        "PYTHONDONTWRITEBYTECODE",
        "1",
    ]
    for name in policy["outputs"]:
        path = str(root / name)
        arguments.extend(("--bind", path, path))
    for name in policy["readonly"]:
        path = root / name
        if path.exists():
            arguments.extend(("--ro-bind", str(path), str(path)))
    for name in (".codex", ".pi"):
        path = Path.home() / name
        if path.exists():
            arguments.extend(("--tmpfs", str(path)))
    for name in ("TMPDIR", "TMP", "TEMP"):
        arguments.extend(("--setenv", name, str(root / policy["temporary"])))
    return [*arguments, "--", *command]


OVERRIDES = (
    "PSX_CC_DRIVER",
    "PSX_GCC",
    "PSX_AS",
    "PSX_MASPSX",
    "MASPSX_PYTHON",
    "ASPSX_VERSION",
    "CFLAGS",
    "CPPFLAGS",
    "LDFLAGS",
    "CMAKE_GENERATOR",
    "CMAKE_TOOLCHAIN_FILE",
    "PSX_PYTHON",
    "PYTHONHOME",
    "LD_PRELOAD",
    "LD_LIBRARY_PATH",
    "CC",
    "CXX",
)
ENVIRONMENT_METADATA = frozenset(
    {"_", "SHLVL", "CODEX_SESSION_ID", "CODEX_THREAD_ID", "PYTEST_CURRENT_TEST"}
)


def _capture_inputs(root: Path, target: str, paths: list[str]) -> dict:
    if any(name in os.environ for name in OVERRIDES):
        raise ValueError("execution context does not support toolchain overrides")
    tools = {}
    for name in ("cmake", "ninja", "python3", "git"):
        resolved = shutil.which(name)
        if resolved is None:
            raise ValueError(f"execution context requires installed {name}")
        path = Path(resolved).resolve(strict=True)
        tools[name] = {"path": str(path), "state": file_state(path)}
    splat = SplatToolchain(repo_layout(root))
    launcher = splat.executable
    python = splat.python
    if file_state(launcher) is None or not os.access(launcher, os.X_OK):
        raise ValueError("execution context requires installed Splat launcher")
    # ponytail: direct project-Python shebang only; other launchers need an owner.
    if launcher.read_bytes().partition(b"\n")[0] != os.fsencode(f"#!{python}"):
        raise ValueError("execution context requires project-Python Splat launcher")
    config = root / ".venv/pyvenv.cfg"
    if file_state(config) is None:
        raise ValueError("execution context requires project Python configuration")
    tools[".venv/bin/python"] = {
        "path": str(python.resolve(strict=True)),
        "state": file_state(python.resolve(strict=True)),
    }
    inputs = input_state(root, target, set(paths))
    inputs[launcher.relative_to(root).as_posix()] = file_state(launcher)
    inputs[".venv/pyvenv.cfg"] = file_state(config)
    # Installed imports and .pth/editable hooks are runtime inputs, not just the
    # source submodules. Bytecode caches are derived execution output.
    for path in (root / ".venv/lib").rglob("*"):
        if path.is_file() and "__pycache__" not in path.parts:
            inputs[path.relative_to(root).as_posix()] = file_state(path)
    inputs["out/index/reverse.sqlite"] = file_state(root / "out/index/reverse.sqlite")
    index = git_index_backup(root)
    return {
        "root": str(root.resolve()),
        "inputs": inputs,
        # Launcher/session bookkeeping varies across independent review runs.
        "environment_digest": digest(
            {
                name: value
                for name, value in os.environ.items()
                if name not in ENVIRONMENT_METADATA
            }
        ),
        "tools": tools,
        "python": {"path": sys.executable, "version": sys.version},
        "git_index": None
        if index is None
        else {
            "path": str(index.path),
            "state": file_state(index.path),
        },
    }
