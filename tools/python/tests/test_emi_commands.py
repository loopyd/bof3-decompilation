from __future__ import annotations

from pathlib import Path

from rebof3.commands import emi as emi_command


def test_emi_unpack_command_runs_shared_operation(
    monkeypatch, capsys, tmp_path: Path
) -> None:
    calls: list[dict[str, Path]] = []

    def fake_emi_unpack(**kwargs) -> int:
        calls.append(kwargs)
        return 2

    monkeypatch.setattr(emi_command, "emi_unpack", fake_emi_unpack)

    result = emi_command.main(
        [
            "emi-unpack",
            "--input-dir",
            str(tmp_path / "build" / "extracted"),
            "--output-dir",
            str(tmp_path / "out" / "emi_raw"),
        ]
    )

    assert result == 0
    assert calls[0]["extracted_dir"] == tmp_path / "build" / "extracted"
    assert calls[0]["raw_emi_dir"] == tmp_path / "out" / "emi_raw"
    assert "unpacked 2 EMI archives" in capsys.readouterr().out


def test_emi_pack_command_runs_shared_operation(
    monkeypatch, capsys, tmp_path: Path
) -> None:
    calls: list[dict[str, Path]] = []

    def fake_emi_pack(**kwargs) -> int:
        calls.append(kwargs)
        return 1

    monkeypatch.setattr(emi_command, "emi_pack", fake_emi_pack)

    result = emi_command.main(
        [
            "emi-pack",
            "--input-dir",
            str(tmp_path / "out" / "emi_raw"),
            "--output-dir",
            str(tmp_path / "build" / "extracted"),
        ]
    )

    assert result == 0
    assert calls[0]["raw_emi_dir"] == tmp_path / "out" / "emi_raw"
    assert calls[0]["extracted_dir"] == tmp_path / "build" / "extracted"
    assert "packed 1 EMI archives" in capsys.readouterr().out
