from __future__ import annotations

from collections.abc import Sequence
from pathlib import Path

from rebof3.commands import ghidra as ghidra_command
from rebof3.ghidra import (
    DEFAULT_IMPORT_STAGING,
    DEFAULT_SYMBOL_EXPORT_SCRIPT,
    GhidraProjectImportResult,
    GhidraSymbolExportResult,
    build_analyze_headless_import_commands,
    build_analyze_headless_symbol_export_command,
    export_ghidra_symbols,
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


def test_build_analyze_headless_symbol_export_command_uses_defaults(
    tmp_path: Path,
) -> None:
    ghidra_home = make_ghidra_home(tmp_path)
    output_path = tmp_path / "out" / "inventory" / "raw_ghidra_export.json"

    command = build_analyze_headless_symbol_export_command(
        ghidra_home=ghidra_home,
        project_dir=tmp_path / "project",
        project_name="bof3",
        output_path=output_path,
    )

    assert command == (
        str(ghidra_home / "support" / "analyzeHeadless"),
        str((tmp_path / "project").resolve()),
        "bof3",
        "-process",
        "-recursive",
        "-scriptPath",
        str(DEFAULT_SYMBOL_EXPORT_SCRIPT.resolve().parent),
        "-postScript",
        DEFAULT_SYMBOL_EXPORT_SCRIPT.name,
        str(output_path.resolve()),
        "/",
        "-noanalysis",
    )


def test_export_ghidra_symbols_runs_command_with_injected_runner(
    tmp_path: Path,
) -> None:
    ghidra_home = make_ghidra_home(tmp_path)
    script_path = tmp_path / "scripts" / "ExportSymbolsJson.java"
    script_path.parent.mkdir()
    script_path.write_text("// script\n", encoding="utf-8")
    output_path = tmp_path / "raw_ghidra_export.json"
    calls: list[tuple[str, ...]] = []

    def runner(command: Sequence[str]) -> int:
        calls.append(tuple(command))
        return 0

    result = export_ghidra_symbols(
        ghidra_home=ghidra_home,
        project_dir=tmp_path / "project",
        project_name="bof3",
        output_path=output_path,
        script_path=script_path,
        process="/boot",
        recursive=False,
        runner=runner,
    )

    assert result.output_path == output_path.resolve()
    assert calls == [result.command]
    assert calls[0] == (
        str(ghidra_home / "support" / "analyzeHeadless"),
        str((tmp_path / "project").resolve()),
        "bof3",
        "-process",
        "boot",
        "-scriptPath",
        str(script_path.parent.resolve()),
        "-postScript",
        script_path.name,
        str(output_path.resolve()),
        "/boot",
        "-noanalysis",
    )


def test_build_symbol_export_command_splits_project_folder_paths(
    tmp_path: Path,
) -> None:
    ghidra_home = make_ghidra_home(tmp_path)
    output_path = tmp_path / "raw.json"

    command = build_analyze_headless_symbol_export_command(
        ghidra_home=ghidra_home,
        project_dir=tmp_path / "project",
        project_name="bof3",
        output_path=output_path,
        process="/bins/BATTLE/BATTLE/3.bin",
        recursive=False,
    )

    assert command[:5] == (
        str(ghidra_home / "support" / "analyzeHeadless"),
        str((tmp_path / "project").resolve()),
        "bof3/bins/BATTLE/BATTLE",
        "-process",
        "3.bin",
    )


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
    assert command[4:6] == (str((tmp_path / "relative.bin").resolve()), "-overwrite")


def test_build_analyze_headless_import_commands_stages_named_imports(
    tmp_path: Path,
) -> None:
    ghidra_home = make_ghidra_home(tmp_path)
    manifest_path = write_manifest(tmp_path)
    staging_dir = tmp_path / "staging"

    command = build_analyze_headless_import_commands(
        ghidra_home=ghidra_home,
        manifest=manifest_path,
        project_dir=tmp_path / "project",
        project_name="bof3",
        staging_dir=staging_dir,
    )[0]

    staged_path = staging_dir.resolve() / "bins" / "AREA01" / "overlay_e00.bin"
    assert command[4:6] == (str(staged_path), "-overwrite")
    assert staged_path.exists()
    assert not staged_path.is_symlink()
    assert staged_path.samefile(tmp_path / "payload.bin")


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
            "staging_dir": DEFAULT_IMPORT_STAGING,
            "script_path": tmp_path / "scripts",
            "analyze": False,
        }
    ]
    assert "imported: 2" in capsys.readouterr().out


def test_ghidra_export_symbols_cli_dispatches_operation(
    monkeypatch,
    tmp_path: Path,
    capsys,
) -> None:
    calls: list[dict[str, object]] = []

    def fake_export_ghidra_symbols(**kwargs: object) -> GhidraSymbolExportResult:
        calls.append(kwargs)
        return GhidraSymbolExportResult(
            output_path=tmp_path / "raw_ghidra_export.json",
            command=(),
        )

    monkeypatch.setattr(
        ghidra_command,
        "export_ghidra_symbols",
        fake_export_ghidra_symbols,
    )

    result = ghidra_command.main(
        [
            "export-symbols",
            "--ghidra-home",
            str(tmp_path / "ghidra"),
            "--project-dir",
            str(tmp_path / "project"),
            "--project-name",
            "bof3",
            "--output",
            str(tmp_path / "raw.json"),
            "--script-path",
            str(tmp_path / "scripts" / "ExportSymbolsJson.java"),
            "--process",
            "/boot",
            "--no-recursive",
        ]
    )

    assert result == 0
    assert calls == [
        {
            "ghidra_home": tmp_path / "ghidra",
            "project_dir": tmp_path / "project",
            "project_name": "bof3",
            "output_path": tmp_path / "raw.json",
            "script_path": tmp_path / "scripts" / "ExportSymbolsJson.java",
            "process": "/boot",
            "recursive": False,
        }
    ]
    assert f"exported: {tmp_path / 'raw_ghidra_export.json'}" in capsys.readouterr().out
