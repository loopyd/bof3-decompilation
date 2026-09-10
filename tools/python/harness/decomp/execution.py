"""Pinned parent-only lift gates in a legacy-compiler-compatible namespace sandbox."""

from __future__ import annotations

from pathlib import Path
import shutil

from harness.common.execution import capture
from harness.common.inputs import file_state
from harness.context.capabilities import verify_policy


def capture_context(root: Path, target: str, paths: list[str]) -> dict:
    context = capture(root, {"targets": [target], "allowed_paths": paths})
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
