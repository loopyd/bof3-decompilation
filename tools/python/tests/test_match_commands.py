from __future__ import annotations

import json
import zipfile
from pathlib import Path

from rebof3.commands import ghidra as ghidra_command
from rebof3.commands import inventory as inventory_command
from rebof3.commands import match as match_command
from rebof3.jsonio import read_json


def write_raw_export(path: Path) -> None:
    path.write_text(
        json.dumps(
            {
                "project_name": "bof3_main",
                "rows": [
                    {
                        "kind": "function",
                        "program_path": "/SLUS_004.22.17",
                        "address": "80162d00",
                        "name": "emi_ready",
                        "type_spec": "bool emi_ready(void)",
                        "body_min": "80162d00",
                        "body_max": "80162d1f",
                        "namespace": "Global",
                        "name_source": "USER_DEFINED",
                        "is_thunk": False,
                    }
                ],
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )


def seed_function_index(tmp_path: Path) -> tuple[Path, Path]:
    raw_export = tmp_path / "raw.json"
    function_index_out = tmp_path / "functions.json"
    write_raw_export(raw_export)
    result = inventory_command.main(
        [
            "import-ghidra-symbols",
            str(raw_export),
            "--index-out",
            str(tmp_path / "index.json"),
            "--function-index-out",
            str(function_index_out),
            "--function-index-tsv-out",
            str(tmp_path / "functions.tsv"),
            "--md-out",
            str(tmp_path / "index.md"),
            "--program-output-dir",
            str(tmp_path / "programs"),
        ]
    )
    assert result == 0
    return function_index_out, tmp_path / "workspaces"


def test_match_init_build_diff_and_report_use_explicit_workspace_outputs(
    tmp_path: Path,
) -> None:
    function_index, workspace_root = seed_function_index(tmp_path)
    expected_artifact = tmp_path / "expected.txt"
    actual_artifact = tmp_path / "actual.txt"
    expected_artifact.write_text("alpha\nbeta\n", encoding="utf-8")
    actual_artifact.write_text("alpha\ngamma\n", encoding="utf-8")

    build_cwd = tmp_path / "build-cwd"
    build_cwd.mkdir()
    build_command = (
        'python3 -c "from pathlib import Path; '
        "Path('build-output.txt').write_text('ok\\n', encoding='utf-8')\""
    )

    init_result = match_command.main(
        [
            "init",
            "--function-index",
            str(function_index),
            "--workspace-root",
            str(workspace_root),
            "--program",
            "/boot/SLUS_004.22",
            "--entry",
            "0x80162d00",
            "--build-command",
            build_command,
            "--build-cwd",
            str(build_cwd),
            "--expected-artifact",
            str(expected_artifact),
            "--actual-artifact",
            str(actual_artifact),
        ]
    )

    assert init_result == 0

    workspace_path = (
        workspace_root / "boot_slus_004_22" / "0x80162d00" / "workspace.json"
    )
    workspace_payload = read_json(workspace_path)
    assert workspace_payload["function"]["program_path"] == "/boot/SLUS_004.22"
    assert workspace_payload["inputs"]["expected_artifact"] == str(
        expected_artifact.resolve()
    )

    build_result = match_command.main(
        [
            "build",
            "--workspace",
            str(workspace_path),
        ]
    )

    assert build_result == 0
    build_status = read_json(workspace_path.parent / "build.json")
    assert build_status["succeeded"] is True
    assert (build_cwd / "build-output.txt").is_file()

    diff_result = match_command.main(
        [
            "diff",
            "--workspace",
            str(workspace_path),
        ]
    )

    assert diff_result == 0
    diff_payload = read_json(workspace_path.parent / "diff.json")
    assert diff_payload["status"] == "different"
    assert "-beta" in (diff_payload["diff_excerpt"] or "")

    report_result = match_command.main(
        [
            "report",
            "--match-root",
            str(workspace_root),
            "--output-json",
            str(tmp_path / "report.json"),
            "--output-tsv",
            str(tmp_path / "report.tsv"),
        ]
    )

    assert report_result == 0
    report_payload = read_json(tmp_path / "report.json")
    assert report_payload["count"] == 1
    assert report_payload["rows"][0]["status"] == "different"
    assert "different" in (tmp_path / "report.tsv").read_text(encoding="utf-8")


def test_match_build_requires_configured_command(tmp_path: Path) -> None:
    function_index, workspace_root = seed_function_index(tmp_path)
    init_result = match_command.main(
        [
            "init",
            "--function-index",
            str(function_index),
            "--workspace-root",
            str(workspace_root),
            "--program",
            "/boot/SLUS_004.22",
            "--entry",
            "0x80162d00",
        ]
    )
    assert init_result == 0

    workspace_path = (
        workspace_root / "boot_slus_004_22" / "0x80162d00" / "workspace.json"
    )

    try:
        match_command.main(["build", "--workspace", str(workspace_path)])
    except ValueError as exc:
        assert "no build command configured" in str(exc)
    else:
        raise AssertionError("expected ValueError for missing build command")


def test_ghidra_install_extensions_installs_directory_and_archive(
    tmp_path: Path,
) -> None:
    extension_dir = tmp_path / "SampleExtension"
    extension_dir.mkdir()
    (extension_dir / "extension.properties").write_text(
        "name=SampleExtension\nversion=1.0\n",
        encoding="utf-8",
    )
    archive_path = tmp_path / "ArchiveExtension.zip"
    with zipfile.ZipFile(archive_path, "w") as archive:
        archive.writestr(
            "ArchiveExtension/extension.properties",
            "name=ArchiveExtension\nversion=1.0\n",
        )

    user_dir = tmp_path / "ghidra-user" / ".ghidra_11.4"
    result = ghidra_command.main(
        [
            "install-extensions",
            str(extension_dir),
            str(archive_path),
            "--user-dir",
            str(user_dir),
        ]
    )

    assert result == 0
    assert (
        user_dir / "Extensions" / "SampleExtension" / "extension.properties"
    ).is_file()
    assert (
        user_dir / "Extensions" / "ArchiveExtension" / "extension.properties"
    ).is_file()


def test_ghidra_bootstrap_reports_missing_inputs(tmp_path: Path, capsys) -> None:
    result = ghidra_command.main(
        [
            "bootstrap",
            "--slus",
            str(tmp_path / "SLUS_004.22"),
            "--logo",
            str(tmp_path / "LOGO.EXE"),
            "--emi-root",
            str(tmp_path / "BIN"),
            "--output-dir",
            str(tmp_path / "ghidra-bootstrap"),
        ]
    )

    assert result == 1
    output = capsys.readouterr().out
    assert "missing Ghidra bootstrap inputs:" in output
    assert "--slus:" in output
    assert "--logo:" in output
    assert "--emi-root:" in output
