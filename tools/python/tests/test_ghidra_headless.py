from __future__ import annotations

from collections.abc import Sequence
from pathlib import Path

from rebof3.commands import ghidra as ghidra_command
from rebof3.ghidra import (
    GhidraProjectImportResult,
    build_analyze_headless_import_commands,
    import_ghidra_project,
)
from rebof3.jsonio import write_json


def make_ghidra_home(tmp_path: Path) -> Path:
    ghidra_home = tmp_path / "ghidra"
    support_dir = ghidra_home / "support"
    support_dir.mkdir(parents=True)
    (support_dir / "analyzeHeadless").write_text("#!/bin/sh\n", encoding="utf-8")
    return ghidra_home


def write_manifest(tmp_path: Path) -> Path:
    binary_path = tmp_path / "payload.bin"
    binary_path.write_bytes(b"data")
    manifest_path = tmp_path / "ghidra_import_manifest.json"
    write_json(
        manifest_path,
        {
            "analyze": False,
            "imports": [
                {
                    "payload_path": str(binary_path),
                    "program_name": "overlay_e00.bin",
                    "project_folder_path": "/bins/AREA01",
                    "loader": {
                        "compiler": "default",
                        "loader_args": [
                            {"name": "-loader-baseAddr", "value": "0x80010000"},
                            {"name": "-loader-blockName", "value": "overlay"},
                        ],
                        "loader_mode": "raw",
                        "loader_name": "BinaryLoader",
                        "processor": "PSX:LE:32:default",
                    },
                }
            ],
        },
    )
    return manifest_path


def test_build_analyze_headless_import_commands_uses_manifest_entries(
    tmp_path: Path,
) -> None:
    ghidra_home = make_ghidra_home(tmp_path)
    manifest_path = write_manifest(tmp_path)
    commands = build_analyze_headless_import_commands(
        ghidra_home=ghidra_home,
        manifest=manifest_path,
        project_dir=tmp_path / "project",
        project_name="bof3",
    )

    assert commands == [
        (
            str(ghidra_home / "support" / "analyzeHeadless"),
            str((tmp_path / "project").resolve()),
            "bof3/bins/AREA01",
            "-import",
            str(tmp_path / "payload.bin"),
            "-programName",
            "overlay_e00.bin",
            "-overwrite",
            "-processor",
            "PSX:LE:32:default",
            "-cspec",
            "default",
            "-loader",
            "BinaryLoader",
            "-loader-baseAddr",
            "0x80010000",
            "-loader-blockName",
            "overlay",
            "-noanalysis",
        )
    ]


def test_import_ghidra_project_runs_commands_with_injected_runner(
    tmp_path: Path,
) -> None:
    ghidra_home = make_ghidra_home(tmp_path)
    manifest_path = write_manifest(tmp_path)
    calls: list[tuple[str, ...]] = []

    def runner(command: Sequence[str]) -> int:
        calls.append(tuple(command))
        return 0

    result = import_ghidra_project(
        ghidra_home=ghidra_home,
        manifest=manifest_path,
        project_dir=tmp_path / "project",
        project_name="bof3",
        analyze=True,
        runner=runner,
    )

    assert result.imported_count == 1
    assert calls == result.commands
    assert "-noanalysis" not in calls[0]


def test_build_analyze_headless_import_commands_accepts_path_and_name(
    tmp_path: Path,
) -> None:
    ghidra_home = make_ghidra_home(tmp_path)
    (tmp_path / "relative.bin").write_bytes(b"data")
    manifest_path = tmp_path / "manifest.json"
    write_json(
        manifest_path,
        {
            "imports": [
                {
                    "path": "relative.bin",
                    "name": "custom.bin",
                    "project_folder_path": "boot",
                }
            ],
        },
    )

    command = build_analyze_headless_import_commands(
        ghidra_home=ghidra_home,
        manifest=manifest_path,
        project_dir=tmp_path / "project",
        project_name="bof3",
    )[0]

    assert command[2] == "bof3/boot"
    assert command[4:7] == (
        str((tmp_path / "relative.bin").resolve()),
        "-programName",
        "custom.bin",
    )


def test_ghidra_import_project_cli_dispatches_operation(
    monkeypatch,
    tmp_path: Path,
    capsys,
) -> None:
    calls: list[dict[str, object]] = []

    def fake_import_ghidra_project(**kwargs: object) -> GhidraProjectImportResult:
        calls.append(kwargs)
        return GhidraProjectImportResult(imported_count=2, commands=[])

    monkeypatch.setattr(
        ghidra_command,
        "import_ghidra_project",
        fake_import_ghidra_project,
    )

    result = ghidra_command.main(
        [
            "import-project",
            "--ghidra-home",
            str(tmp_path / "ghidra"),
            "--manifest",
            str(tmp_path / "manifest.json"),
            "--project-dir",
            str(tmp_path / "project"),
            "--project-name",
            "bof3",
            "--script-path",
            str(tmp_path / "scripts"),
            "--no-analyze",
        ]
    )

    assert result == 0
    assert calls == [
        {
            "ghidra_home": tmp_path / "ghidra",
            "manifest": tmp_path / "manifest.json",
            "project_dir": tmp_path / "project",
            "project_name": "bof3",
            "script_path": tmp_path / "scripts",
            "analyze": False,
        }
    ]
    assert "imported: 2" in capsys.readouterr().out
