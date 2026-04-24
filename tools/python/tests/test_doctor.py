from __future__ import annotations

from pathlib import Path

import pytest

from rebof3.commands.doctor import build_parser
from rebof3.doctor import DoctorCheck, doctor_exit_code, run_doctor
from rebof3.paths import repo_layout


def seed_executable(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("", encoding="utf-8")
    path.chmod(0o755)


def seed_open_layout(layout) -> None:
    for tool_name in ("bof3-disk", "emi-ex", "maspsx", "objdiff", "mipsmatch"):
        (layout.third_party_dir / tool_name).mkdir(parents=True, exist_ok=True)
        (layout.third_party_dir / tool_name / "README.md").write_text(
            "", encoding="utf-8"
        )
    for tool_path in (
        layout.bof3_disk_bin,
        layout.emi_ex_bin,
        layout.objdiff_bin,
        layout.mipsmatch_bin,
    ):
        seed_executable(tool_path)
    for tool_name in (
        "mipsel-none-elf-gcc",
        "mipsel-none-elf-objdump",
        "mipsel-none-elf-nm",
    ):
        seed_executable(layout.psn00b_toolchain_root / "bin" / tool_name)
    layout.gcc272_psx_root.mkdir(parents=True, exist_ok=True)
    (layout.gcc272_psx_root / "gcc").write_text("", encoding="utf-8")
    (layout.gcc272_psx_root / "gcc").chmod(0o755)
    (layout.aspsx_psyq_root / "psyq4.0").mkdir(parents=True, exist_ok=True)
    (layout.aspsx_psyq_root / "psyq4.0" / "ASPSX.EXE").write_text("", encoding="utf-8")


def seed_layout(layout) -> None:
    for tool_name in (
        "bof3-disk",
        "emi-ex",
        "maspsx",
        "objdiff",
        "mipsmatch",
        "bof3-ghidra",
        "ghidra-mcp",
        "asm-differ",
        "m2c",
        "decomp-permuter",
    ):
        (layout.third_party_dir / tool_name).mkdir(parents=True, exist_ok=True)
        (layout.third_party_dir / tool_name / "README.md").write_text(
            "", encoding="utf-8"
        )
    for tool_path in (
        layout.bof3_disk_bin,
        layout.emi_ex_bin,
        layout.objdiff_bin,
        layout.mipsmatch_bin,
    ):
        seed_executable(tool_path)
    layout.disc_dir.mkdir(parents=True, exist_ok=True)
    (layout.disc_dir / "game.cue").write_text(
        'FILE "game.bin" BINARY\n', encoding="utf-8"
    )
    for tool_name in (
        "mipsel-none-elf-gcc",
        "mipsel-none-elf-objdump",
        "mipsel-none-elf-nm",
    ):
        seed_executable(layout.psn00b_toolchain_root / "bin" / tool_name)
    layout.gcc272_psx_root.mkdir(parents=True, exist_ok=True)
    (layout.gcc272_psx_root / "gcc").write_text("", encoding="utf-8")
    (layout.gcc272_psx_root / "gcc").chmod(0o755)
    (layout.psyq_root / "include").mkdir(parents=True, exist_ok=True)
    (layout.psyq_root / "include" / "libgpu.h").write_text("", encoding="utf-8")
    (layout.aspsx_psyq_root / "psyq4.0").mkdir(parents=True, exist_ok=True)
    (layout.aspsx_psyq_root / "psyq4.0" / "ASPSX.EXE").write_text("", encoding="utf-8")
    layout.ghidra_manifest_path.parent.mkdir(parents=True, exist_ok=True)
    layout.ghidra_manifest_path.write_text("{}", encoding="utf-8")
    layout.inventory_ghidra_symbols_index_path.parent.mkdir(parents=True, exist_ok=True)
    layout.inventory_ghidra_symbols_index_path.write_text("{}", encoding="utf-8")
    layout.inventory_ghidra_function_index_path.write_text("{}", encoding="utf-8")


def test_doctor_passes_for_seeded_layout(monkeypatch, tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)
    seed_layout(layout)

    monkeypatch.setattr("rebof3.doctor.shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr(
        "rebof3.doctor.find_psyq_source",
        lambda **_: type(
            "PsyqSource", (), {"kind": "tree", "path": tmp_path / "psyq-source"}
        )(),
    )

    checks = run_doctor(layout=layout, profile="full")

    assert doctor_exit_code(checks) == 0
    assert all(check.status == "ok" for check in checks)
    assert layout.psyq_root == tmp_path / "toolchains" / "psyq" / "4.7"


def test_doctor_open_profile_passes_for_seeded_open_layout(
    monkeypatch, tmp_path: Path
) -> None:
    layout = repo_layout(tmp_path)
    seed_open_layout(layout)

    monkeypatch.setattr("rebof3.doctor.shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr("rebof3.doctor.find_psyq_source", lambda **_: None)

    checks = run_doctor(layout=layout, profile="open")

    assert doctor_exit_code(checks) == 0
    assert all(check.name != "inputs/disc" for check in checks)
    assert all(check.name != "inputs/local-psyq-source" for check in checks)
    assert all(check.name != "toolchains/psyq" for check in checks)
    assert all(check.name != "out/ghidra-bootstrap" for check in checks)
    assert any(check.name == "tools/bof3-disk" for check in checks)


def test_doctor_workspace_profile_checks_only_workspace_prerequisites(
    monkeypatch, tmp_path: Path
) -> None:
    layout = repo_layout(tmp_path)
    for tool_name in ("bof3-disk", "emi-ex", "maspsx", "objdiff", "mipsmatch"):
        (layout.third_party_dir / tool_name).mkdir(parents=True, exist_ok=True)
        (layout.third_party_dir / tool_name / "README.md").write_text(
            "", encoding="utf-8"
        )

    monkeypatch.setattr("rebof3.doctor.shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr("rebof3.doctor.find_psyq_source", lambda **_: None)

    checks = run_doctor(layout=layout, profile="workspace")

    assert doctor_exit_code(checks) == 0
    assert any(check.name == "command/uv" for check in checks)
    assert any(check.name == "third_party/maspsx" for check in checks)
    assert all(not check.name.startswith("tools/") for check in checks)
    assert all(not check.name.startswith("toolchains/") for check in checks)


def test_doctor_decomp_profile_requires_match_tools_and_ghidra_indexes(
    monkeypatch, tmp_path: Path
) -> None:
    layout = repo_layout(tmp_path)
    seed_layout(layout)

    monkeypatch.setattr("rebof3.doctor.shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr(
        "rebof3.doctor.find_psyq_source",
        lambda **_: type(
            "PsyqSource", (), {"kind": "tree", "path": tmp_path / "psyq-source"}
        )(),
    )

    checks = run_doctor(layout=layout, profile="decomp")

    assert doctor_exit_code(checks) == 0
    assert any(check.name == "tools/objdiff-cli" for check in checks)
    assert any(check.name == "tools/mipsmatch" for check in checks)
    assert any(check.name == "out/ghidra-bootstrap" for check in checks)
    assert any(check.name == "out/ghidra-function-index" for check in checks)


def test_doctor_decomp_profile_fails_without_ghidra_function_index(
    monkeypatch, tmp_path: Path
) -> None:
    layout = repo_layout(tmp_path)
    seed_layout(layout)
    layout.inventory_ghidra_function_index_path.unlink()

    monkeypatch.setattr("rebof3.doctor.shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr(
        "rebof3.doctor.find_psyq_source",
        lambda **_: type(
            "PsyqSource", (), {"kind": "tree", "path": tmp_path / "psyq-source"}
        )(),
    )

    checks = run_doctor(layout=layout, profile="decomp")

    assert any(
        check.name == "out/ghidra-function-index"
        and check.status == "missing"
        and check.required
        for check in checks
    )
    assert doctor_exit_code(checks) == 1


def test_doctor_full_profile_requires_ghidra_outputs(
    monkeypatch, tmp_path: Path
) -> None:
    layout = repo_layout(tmp_path)
    seed_layout(layout)
    layout.ghidra_manifest_path.unlink()

    monkeypatch.setattr("rebof3.doctor.shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr(
        "rebof3.doctor.find_psyq_source",
        lambda **_: type(
            "PsyqSource", (), {"kind": "tree", "path": tmp_path / "psyq-source"}
        )(),
    )

    checks = run_doctor(layout=layout, profile="full")

    assert any(
        check.name == "out/ghidra-bootstrap"
        and check.status == "missing"
        and check.required
        for check in checks
    )
    assert doctor_exit_code(checks) == 1


def test_doctor_ghidra_profile_does_not_require_match_tool_binaries(
    monkeypatch, tmp_path: Path
) -> None:
    layout = repo_layout(tmp_path)
    seed_layout(layout)
    layout.objdiff_bin.unlink()
    layout.mipsmatch_bin.unlink()

    monkeypatch.setattr("rebof3.doctor.shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr(
        "rebof3.doctor.find_psyq_source",
        lambda **_: type(
            "PsyqSource", (), {"kind": "tree", "path": tmp_path / "psyq-source"}
        )(),
    )

    checks = run_doctor(layout=layout, profile="ghidra")

    assert doctor_exit_code(checks) == 0
    assert all(check.name != "tools/objdiff-cli" for check in checks)
    assert all(check.name != "tools/mipsmatch" for check in checks)
    assert any(check.name == "out/ghidra-bootstrap" for check in checks)


def test_doctor_fails_when_required_prerequisites_are_missing(
    monkeypatch, tmp_path: Path
) -> None:
    layout = repo_layout(tmp_path)

    def fake_which(name: str) -> str | None:
        if name == "cmake":
            return None
        return f"/usr/bin/{name}"

    monkeypatch.setattr("rebof3.doctor.shutil.which", fake_which)
    monkeypatch.setattr("rebof3.doctor.find_psyq_source", lambda **_: None)

    checks = run_doctor(layout=layout, profile="full")

    assert doctor_exit_code(checks) == 1
    assert any(
        check.name == "command/cmake" and check.status == "missing" for check in checks
    )


def test_doctor_flags_empty_submodule_directories_as_missing(
    monkeypatch, tmp_path: Path
) -> None:
    layout = repo_layout(tmp_path)
    for tool_name in ("bof3-disk", "emi-ex", "maspsx", "objdiff", "mipsmatch"):
        (layout.third_party_dir / tool_name).mkdir(parents=True, exist_ok=True)

    monkeypatch.setattr("rebof3.doctor.shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr("rebof3.doctor.find_psyq_source", lambda **_: None)

    checks = run_doctor(layout=layout, profile="full")

    assert any(
        check.name == "third_party/bof3-disk" and check.status == "missing"
        for check in checks
    )


def test_doctor_flags_missing_pipeline_binaries(monkeypatch, tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)
    seed_open_layout(layout)
    layout.bof3_disk_bin.unlink()

    monkeypatch.setattr("rebof3.doctor.shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr("rebof3.doctor.find_psyq_source", lambda **_: None)

    checks = run_doctor(layout=layout, profile="open")

    assert any(
        check.name == "tools/bof3-disk" and check.status == "missing"
        for check in checks
    )
    assert doctor_exit_code(checks) == 1


def test_doctor_strict_fails_optional_issues() -> None:
    checks = [
        DoctorCheck(
            name="out/example",
            status="missing",
            detail="missing optional output",
            required=False,
        )
    ]

    assert doctor_exit_code(checks) == 0
    assert doctor_exit_code(checks, strict=True) == 1


def test_doctor_parser_defaults_to_full_profile() -> None:
    args = build_parser().parse_args([])

    assert args.profile == "full"
    assert not args.open_profile


def test_doctor_parser_supports_open_alias() -> None:
    args = build_parser().parse_args(["--open"])

    assert args.profile == "full"
    assert args.open_profile


def test_doctor_parser_documents_profile_choices(capsys) -> None:
    with pytest.raises(SystemExit):
        build_parser().parse_args(["--help"])

    help_text = capsys.readouterr().out
    assert "--profile {open,full,ghidra,decomp,workspace}" in help_text
