from __future__ import annotations

import subprocess
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[3]


def run_wrapper(*args: str, cwd: Path) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [str(REPO_ROOT / "bin" / args[0]), *args[1:]],
        cwd=cwd,
        check=False,
        capture_output=True,
        text=True,
    )


def test_configure_wrapper_uses_repo_root_for_presets(tmp_path: Path) -> None:
    result = run_wrapper("configure", "--list-presets", cwd=tmp_path)

    assert result.returncode == 0, result.stderr
    assert '"default"' in result.stdout


def test_build_wrapper_uses_repo_root_for_presets(tmp_path: Path) -> None:
    result = run_wrapper("build", "--list-presets", cwd=tmp_path)

    assert result.returncode == 0, result.stderr
    assert '"default"' in result.stdout


def test_bof3_wrapper_help_keeps_compatibility_entrypoint_available(
    tmp_path: Path,
) -> None:
    result = run_wrapper("bof3", "--help", cwd=tmp_path)

    assert result.returncode == 0, result.stderr
    assert "usage: bof3" in result.stdout
    assert "Compatibility entrypoint for the legacy aggregate CLI" in result.stdout


def test_inventory_wrapper_help_still_exposes_legacy_alias(tmp_path: Path) -> None:
    result = run_wrapper("inventory", "--help", cwd=tmp_path)

    assert result.returncode == 0, result.stderr
    assert "usage: inventory" in result.stdout


def test_disk_extract_wrapper_help_is_available(tmp_path: Path) -> None:
    result = run_wrapper("disk-extract", "--help", cwd=tmp_path)

    assert result.returncode == 0, result.stderr
    assert "usage: disk-extract" in result.stdout


def test_emi_unpack_wrapper_help_is_available(tmp_path: Path) -> None:
    result = run_wrapper("emi-unpack", "--help", cwd=tmp_path)

    assert result.returncode == 0, result.stderr
    assert "usage: emi-unpack" in result.stdout


def test_inventory_build_wrapper_help_is_available(tmp_path: Path) -> None:
    result = run_wrapper("inventory-build", "--help", cwd=tmp_path)

    assert result.returncode == 0, result.stderr
    assert "usage: inventory build" in result.stdout


def test_inventory_import_ghidra_symbols_wrapper_help_is_available(
    tmp_path: Path,
) -> None:
    result = run_wrapper("inventory-import-ghidra-symbols", "--help", cwd=tmp_path)

    assert result.returncode == 0, result.stderr
    assert "usage: inventory import-ghidra-symbols" in result.stdout


def test_ghidra_install_extensions_wrapper_help_is_available(tmp_path: Path) -> None:
    result = run_wrapper("ghidra-install-extensions", "--help", cwd=tmp_path)

    assert result.returncode == 0, result.stderr
    assert "usage: ghidra install-extensions" in result.stdout


def test_match_init_wrapper_help_is_available(tmp_path: Path) -> None:
    result = run_wrapper("match-init", "--help", cwd=tmp_path)

    assert result.returncode == 0, result.stderr
    assert "usage: match init" in result.stdout


def test_match_report_wrapper_help_is_available(tmp_path: Path) -> None:
    result = run_wrapper("match-report", "--help", cwd=tmp_path)

    assert result.returncode == 0, result.stderr
    assert "usage: match report" in result.stdout
