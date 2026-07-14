from __future__ import annotations

from pathlib import Path
import struct
import subprocess

from harness.assets.str_media import (
    convert_str,
    inspect_str,
    raw_sector_bytes,
    validate_str,
)


def _sector(submode: int, coding: int, payload: bytes = b"") -> bytes:
    header = bytes((1, 1, submode, coding))
    return header + header + payload.ljust(2328, b"\0")


def _video(frame: int, chunk: int, chunks: int, *, eof: bool = False) -> bytes:
    payload = bytearray(2328)
    payload[:4] = b"\x60\x01\x01\x80"
    struct.pack_into("<HHI", payload, 4, chunk, chunks, frame)
    return _sector(0xC2 if eof else 0x42, 0x80, payload)


def test_inspect_str_reports_complete_frames_and_audio_duration() -> None:
    data = b"".join((_video(1, 0, 2), _sector(0x64, 1), _video(1, 1, 2, eof=True)))

    result = inspect_str(data)

    assert result["sector_count"] == 3
    assert result["frame_count"] == 1
    assert result["frame_gaps"] == []
    assert result["incomplete_frames"] == []
    assert result["eof_sectors"] == [2]
    assert result["audio_streams"][0]["samples"] == 2016


def test_raw_wrapper_preserves_every_2336_byte_sector() -> None:
    source = _video(1, 0, 1) + _sector(0x64, 1)

    wrapped = raw_sector_bytes(source)

    assert len(wrapped) == 2 * 2352
    assert wrapped[16:2352] == source[:2336]
    assert wrapped[2352 + 16 :] == source[2336:]


def test_validate_is_observational_without_fps_and_verdict_with_fps(
    tmp_path: Path,
) -> None:
    source = tmp_path / "sample.str"
    source.write_bytes(_video(1, 0, 1) + _sector(0x64, 1))

    observed = validate_str(source, tmp_path / "observed")
    compared = validate_str(source, tmp_path / "compared", expected_fps=30)

    assert observed["status"] == "observed"
    assert "timing" not in observed
    assert compared["status"] in {"pass", "fail"}
    assert compared["timing"]["tolerance_basis"].startswith("two video frames")
    assert Path(compared["wrapper"]).is_file()


def test_convert_builds_corrected_timing_and_audio_padding_command(
    tmp_path: Path, monkeypatch
) -> None:
    source = tmp_path / "sample.str"
    source.write_bytes(_video(1, 0, 1) + _video(2, 0, 1) + _sector(0x64, 1))
    commands: list[list[str]] = []

    def fake_run(command: list[str], **_: object) -> subprocess.CompletedProcess[str]:
        commands.append(command)
        return subprocess.CompletedProcess(command, 0, "", "")

    monkeypatch.setattr(subprocess, "run", fake_run)
    monkeypatch.setattr("harness.assets.str_media.shutil.which", lambda _: None)

    result = convert_str(
        source,
        tmp_path / "out",
        fps=30,
        ffmpeg="ffmpeg",
        ffprobe="",
    )

    padding = result["audio_padding"]
    assert padding["padding_samples_per_channel"] == 504
    filter_graph = commands[0][commands[0].index("-filter_complex") + 1]
    assert "settb=AVTB,setpts=N/(30*TB),fps=30[video]" in filter_graph
    assert "apad=pad_len=504,atrim=end_sample=2520[main_audio]" in filter_graph
    assert commands[0][commands[0].index("-map") + 1] == "[video]"
    assert commands[0][commands[0].index("-c:v") + 1] == "libx264"
    assert commands[0][commands[0].index("-qp") + 1] == "0"
    assert "-pix_fmt" not in commands[0]
