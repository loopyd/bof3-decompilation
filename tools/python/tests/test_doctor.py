from __future__ import annotations

from pathlib import Path

from rebof3.doctor import doctor_exit_code, run_doctor
from rebof3.paths import repo_layout


def seed_open_layout(layout) -> None:
    for tool_name in ("bof3-disk", "emi-ex", "maspsx", "objdiff", "mipsmatch"):
        (layout.third_party_dir / tool_name).mkdir(parents=True, exist_ok=True)
        (layout.third_party_dir / tool_name / "README.md").write_text(
            "", encoding="utf-8"
        )
    (layout.psn00b_toolchain_root / "bin").mkdir(parents=True, exist_ok=True)
    (layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-gcc").write_text(
        "", encoding="utf-8"
    )
    layout.gcc272_psx_root.mkdir(parents=True, exist_ok=True)
    (layout.gcc272_psx_root / "gcc").write_text("", encoding="utf-8")
    (layout.aspsx_psyq_root / "psyq4.0").mkdir(parents=True, exist_ok=True)
    (layout.aspsx_psyq_root / "psyq4.0" / "ASPSX.EXE").write_text("", encoding="utf-8")


def seed_layout(layout) -> None:
    for tool_name in ("bof3-disk", "emi-ex", "maspsx", "objdiff", "mipsmatch"):
        (layout.third_party_dir / tool_name).mkdir(parents=True, exist_ok=True)
        (layout.third_party_dir / tool_name / "README.md").write_text(
            "", encoding="utf-8"
        )
    layout.disc_dir.mkdir(parents=True, exist_ok=True)
    (layout.disc_dir / "game.cue").write_text(
        'FILE "game.bin" BINARY\n', encoding="utf-8"
    )
    (layout.psn00b_toolchain_root / "bin").mkdir(parents=True, exist_ok=True)
    (layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-gcc").write_text(
        "", encoding="utf-8"
    )
    layout.gcc272_psx_root.mkdir(parents=True, exist_ok=True)
    (layout.gcc272_psx_root / "gcc").write_text("", encoding="utf-8")
    (layout.psyq_root / "include").mkdir(parents=True, exist_ok=True)
    (layout.psyq_root / "include" / "libgpu.h").write_text("", encoding="utf-8")
    (layout.aspsx_psyq_root / "psyq4.0").mkdir(parents=True, exist_ok=True)
    (layout.aspsx_psyq_root / "psyq4.0" / "ASPSX.EXE").write_text("", encoding="utf-8")
    layout.ghidra_manifest_path.parent.mkdir(parents=True, exist_ok=True)
    layout.ghidra_manifest_path.write_text("{}", encoding="utf-8")


def test_doctor_passes_for_seeded_layout(monkeypatch, tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)
    seed_layout(layout)

    monkeypatch.setattr("rebof3.doctor.shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr(
        "rebof3.doctor.find_psyq_source",
        lambda: type(
            "PsyqSource", (), {"kind": "tree", "path": tmp_path / "psyq-source"}
        )(),
    )

    checks = run_doctor(layout=layout)

    assert doctor_exit_code(checks) == 0
    assert all(check.status == "ok" for check in checks)
    assert layout.psyq_root == tmp_path / "toolchains" / "psyq" / "4.7"


def test_doctor_open_profile_passes_for_seeded_open_layout(
    monkeypatch, tmp_path: Path
) -> None:
    layout = repo_layout(tmp_path)
    seed_open_layout(layout)

    monkeypatch.setattr("rebof3.doctor.shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr("rebof3.doctor.find_psyq_source", lambda: None)

    checks = run_doctor(
        layout=layout,
        include_local_inputs=False,
        include_generated_outputs=False,
    )

    assert doctor_exit_code(checks) == 0
    assert all(check.name != "inputs/disc" for check in checks)
    assert all(check.name != "inputs/local-psyq-source" for check in checks)
    assert all(check.name != "toolchains/psyq" for check in checks)
    assert all(check.name != "out/ghidra-bootstrap" for check in checks)


def test_doctor_fails_when_required_prerequisites_are_missing(
    monkeypatch, tmp_path: Path
) -> None:
    layout = repo_layout(tmp_path)

    def fake_which(name: str) -> str | None:
        if name == "cmake":
            return None
        return f"/usr/bin/{name}"

    monkeypatch.setattr("rebof3.doctor.shutil.which", fake_which)
    monkeypatch.setattr("rebof3.doctor.find_psyq_source", lambda: None)

    checks = run_doctor(layout=layout)

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
    monkeypatch.setattr("rebof3.doctor.find_psyq_source", lambda: None)

    checks = run_doctor(layout=layout)

    assert any(
        check.name == "third_party/bof3-disk" and check.status == "missing"
        for check in checks
    )
