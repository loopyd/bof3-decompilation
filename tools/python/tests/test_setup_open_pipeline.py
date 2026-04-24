from __future__ import annotations

from pathlib import Path
from typing import Sequence

from rebof3.pipelines.setup_open import build_setup_open_pipeline


class RecordingExecutor:
    def __init__(self) -> None:
        self.calls: list[tuple[tuple[str, ...], Path]] = []

    def __call__(self, command: Sequence[str], *, cwd: Path) -> None:
        self.calls.append((tuple(command), cwd))


def test_setup_open_pipeline_task_order() -> None:
    pipeline = build_setup_open_pipeline(root=Path("/repo"))

    assert [task.name for task in pipeline.plan()] == [
        "sync-submodules",
        "update-submodules",
        "check-submodules",
        "setup-native-tools",
        "setup-match-tools",
        "setup-open-toolchains",
        "verify-open-doctor",
    ]


def test_setup_open_pipeline_wires_commands_and_doctor_profile() -> None:
    executor = RecordingExecutor()
    root = Path("/repo")
    context = {"workspace": "open"}

    result = build_setup_open_pipeline(root=root, executor=executor).run(context)

    assert result is context
    assert executor.calls == [
        (("git", "submodule", "sync", "--recursive"), root),
        (("git", "submodule", "update", "--init", "--recursive"), root),
        (("git", "submodule", "status", "--recursive"), root),
        ((str(root / "bin" / "setup-native-tools"),), root),
        ((str(root / "bin" / "setup-match-tools"),), root),
        ((str(root / "bin" / "setup-psx-toolchain"),), root),
        ((str(root / "bin" / "setup-aspsx"),), root),
        ((str(root / "bin" / "doctor"), "--profile", "open"), root),
    ]


def test_setup_open_pipeline_passes_force_to_open_toolchain_installers() -> None:
    executor = RecordingExecutor()
    root = Path("/repo")

    build_setup_open_pipeline(root=root, executor=executor, force=True).run()

    assert (str(root / "bin" / "setup-psx-toolchain"), "--force") in [
        command for command, _ in executor.calls
    ]
    assert (str(root / "bin" / "setup-aspsx"), "--force") in [
        command for command, _ in executor.calls
    ]


def test_setup_open_pipeline_uses_required_submodule_paths(tmp_path: Path) -> None:
    (tmp_path / ".gitmodules").write_text(
        """
[submodule "third_party/bof3-disk"]
    path = third_party/bof3-disk
    url = https://example.invalid/bof3-disk.git
[submodule "external/private-assets"]
    path = external/private-assets
    url = https://example.invalid/private-assets.git
""",
        encoding="utf-8",
    )
    executor = RecordingExecutor()

    build_setup_open_pipeline(root=tmp_path, executor=executor).run()

    assert executor.calls[0] == (
        (
            "git",
            "submodule",
            "sync",
            "--recursive",
            "--",
            "third_party/bof3-disk",
        ),
        tmp_path,
    )
    assert "external/private-assets" not in {
        part for command, _ in executor.calls for part in command
    }
