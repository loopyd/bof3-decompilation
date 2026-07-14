from __future__ import annotations

import hashlib
import json
from fractions import Fraction
from pathlib import Path
import shutil
import struct
import subprocess
from typing import Any


XA_SECTOR_SIZE = 2336
RAW_SECTOR_SIZE = 2352
STR_MAGIC = b"\x60\x01\x01\x80"


def inspect_str(data: bytes) -> dict[str, Any]:
    if not data or len(data) % XA_SECTOR_SIZE:
        raise ValueError("STR size must be a non-zero multiple of 2336 bytes")

    frames: dict[int, list[tuple[int, int]]] = {}
    audio: dict[tuple[int, int, int], int] = {}
    duplicate_subheader_errors: list[int] = []
    eof_sectors: list[int] = []
    for sector_index in range(len(data) // XA_SECTOR_SIZE):
        sector = data[
            sector_index * XA_SECTOR_SIZE : (sector_index + 1) * XA_SECTOR_SIZE
        ]
        if sector[:4] != sector[4:8]:
            duplicate_subheader_errors.append(sector_index)
        file_number, channel, submode, coding = sector[:4]
        if submode & 0x80:
            eof_sectors.append(sector_index)
        if sector[8:12] == STR_MAGIC:
            chunk, chunk_count = struct.unpack_from("<HH", sector, 12)
            frame = struct.unpack_from("<I", sector, 16)[0]
            frames.setdefault(frame, []).append((chunk, chunk_count))
        elif submode & 0x04:
            key = (file_number, channel, coding)
            audio[key] = audio.get(key, 0) + 1

    incomplete_frames: list[int] = []
    for frame, chunks in frames.items():
        counts = {count for _, count in chunks}
        expected = next(iter(counts)) if len(counts) == 1 else -1
        if expected < 0 or sorted(chunk for chunk, _ in chunks) != list(
            range(expected)
        ):
            incomplete_frames.append(frame)
    ordered_frames = sorted(frames)
    frame_gaps = (
        sorted(set(range(ordered_frames[0], ordered_frames[-1] + 1)) - set(frames))
        if ordered_frames
        else []
    )

    audio_streams = []
    for (file_number, channel, coding), sector_count in sorted(audio.items()):
        stereo = bool(coding & 0x01)
        sample_rate = 18900 if coding & 0x04 else 37800
        samples_per_sector = 2016 if stereo else 4032
        audio_streams.append(
            {
                "file_number": file_number,
                "channel": channel,
                "coding": coding,
                "channels": 2 if stereo else 1,
                "sample_rate": sample_rate,
                "sector_count": sector_count,
                "samples": sector_count * samples_per_sector,
                "duration_seconds": sector_count * samples_per_sector / sample_rate,
            }
        )
    audio_streams.sort(key=lambda stream: int(stream["sector_count"]), reverse=True)
    return {
        "sector_size": XA_SECTOR_SIZE,
        "sector_count": len(data) // XA_SECTOR_SIZE,
        "frame_count": len(frames),
        "first_frame": ordered_frames[0] if ordered_frames else None,
        "last_frame": ordered_frames[-1] if ordered_frames else None,
        "frame_gaps": frame_gaps,
        "incomplete_frames": sorted(incomplete_frames),
        "duplicate_subheader_errors": duplicate_subheader_errors,
        "eof_sectors": eof_sectors,
        "audio_streams": audio_streams,
    }


def _bcd(value: int) -> int:
    return (value // 10) * 16 + value % 10


def raw_sector_bytes(data: bytes) -> bytes:
    inspection = inspect_str(data)
    sync = b"\x00" + b"\xff" * 10 + b"\x00"
    output = bytearray(int(inspection["sector_count"]) * RAW_SECTOR_SIZE)
    for sector_index in range(int(inspection["sector_count"])):
        lba = sector_index + 150
        header = sync + bytes(
            (_bcd(lba // 4500), _bcd((lba // 75) % 60), _bcd(lba % 75), 2)
        )
        source_start = sector_index * XA_SECTOR_SIZE
        output_start = sector_index * RAW_SECTOR_SIZE
        output[output_start : output_start + 16] = header
        output[output_start + 16 : output_start + RAW_SECTOR_SIZE] = data[
            source_start : source_start + XA_SECTOR_SIZE
        ]
    return bytes(output)


def _probe(path: Path, ffprobe: str) -> dict[str, Any]:
    completed = subprocess.run(
        [
            ffprobe,
            "-v",
            "error",
            "-count_frames",
            "-show_entries",
            "format=duration:stream=index,codec_type,codec_name,time_base,duration,"
            "avg_frame_rate,sample_rate,channels,nb_frames,nb_read_frames,"
            "pix_fmt,color_range,width,height",
            "-of",
            "json",
            str(path),
        ],
        check=True,
        capture_output=True,
        text=True,
    )
    return json.loads(completed.stdout)


def _probe_packet_ends(path: Path, ffprobe: str) -> dict[int, float]:
    completed = subprocess.run(
        [
            ffprobe,
            "-v",
            "error",
            "-show_entries",
            "packet=stream_index,pts_time,duration_time",
            "-of",
            "json",
            str(path),
        ],
        check=True,
        capture_output=True,
        text=True,
    )
    ends: dict[int, float] = {}
    for packet in json.loads(completed.stdout).get("packets", []):
        stream = int(packet["stream_index"])
        end = float(packet.get("pts_time", 0)) + float(packet.get("duration_time", 0))
        ends[stream] = max(ends.get(stream, 0.0), end)
    return ends


def validate_str(
    source: Path,
    output_dir: Path,
    *,
    expected_fps: float | None = None,
    ffprobe: str | None = None,
) -> dict[str, Any]:
    data = source.read_bytes()
    result = inspect_str(data)
    output_dir.mkdir(parents=True, exist_ok=True)
    wrapper = output_dir / f"{source.stem}.raw.str"
    wrapper.write_bytes(raw_sector_bytes(data))
    result.update(
        {
            "schema": "harness.str-validation/v1",
            "source": str(source),
            "source_sha256": hashlib.sha256(data).hexdigest(),
            "wrapper": str(wrapper),
            "status": "observed",
        }
    )
    if expected_fps is not None:
        if expected_fps <= 0:
            raise ValueError("expected FPS must be greater than zero")
        primary_audio = result["audio_streams"][0] if result["audio_streams"] else None
        video_duration = result["frame_count"] / expected_fps
        audio_duration = (
            float(primary_audio["duration_seconds"]) if primary_audio else None
        )
        audio_packet = (
            float(primary_audio["samples"])
            / int(primary_audio["sector_count"])
            / int(primary_audio["sample_rate"])
            if primary_audio
            else 0.0
        )
        tolerance = max(2.0 / expected_fps, 2.0 * audio_packet)
        delta = abs(video_duration - audio_duration) if audio_duration else None
        result["timing"] = {
            "expected_fps": expected_fps,
            "video_duration_seconds": video_duration,
            "primary_audio_duration_seconds": audio_duration,
            "duration_delta_seconds": delta,
            "tolerance_seconds": tolerance,
            "tolerance_basis": "two video frames or two primary XA audio sectors",
        }
        result["status"] = (
            "pass" if delta is not None and delta <= tolerance else "fail"
        )
    if ffprobe:
        result["ffprobe"] = _probe(wrapper, ffprobe)
    manifest = output_dir / "validation.json"
    manifest.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    result["manifest"] = str(manifest)
    return result


def convert_str(
    source: Path,
    output_dir: Path,
    *,
    fps: float,
    output: Path | None = None,
    ffmpeg: str | None = None,
    ffprobe: str | None = None,
) -> dict[str, Any]:
    if fps <= 0:
        raise ValueError("FPS must be greater than zero")
    ffmpeg = ffmpeg or shutil.which("ffmpeg")
    if not ffmpeg:
        raise RuntimeError("ffmpeg is required for STR conversion")
    validation = validate_str(source, output_dir, expected_fps=fps)
    primary_audio = (
        validation["audio_streams"][0] if validation["audio_streams"] else None
    )
    if primary_audio is None:
        raise ValueError("STR conversion requires a primary XA audio stream")
    video_duration = int(validation["frame_count"]) / fps
    sample_rate = int(primary_audio["sample_rate"])
    decoded_samples = int(primary_audio["samples"])
    pad_samples = max(0, round(video_duration * sample_rate) - decoded_samples)
    final_audio_samples = decoded_samples + pad_samples
    destination = output or output_dir / f"{source.stem}_{fps:g}fps.mkv"
    destination.parent.mkdir(parents=True, exist_ok=True)
    ffprobe = ffprobe or shutil.which("ffprobe")
    source_probe = _probe(Path(validation["wrapper"]), ffprobe) if ffprobe else None
    source_video = next(
        (
            stream
            for stream in (source_probe or {}).get("streams", [])
            if stream.get("codec_type") == "video"
        ),
        {},
    )
    video_codec = ["-c:v", "libx264", "-qp", "0", "-preset", "slow"]
    if source_video.get("color_range") in {"pc", "tv"}:
        video_codec.extend(["-color_range", str(source_video["color_range"])])
    command = [
        ffmpeg,
        "-y",
        "-v",
        "error",
        "-f",
        "psxstr",
        "-i",
        str(validation["wrapper"]),
        "-filter_complex",
        f"[0:v:0]settb=AVTB,setpts=N/({fps:g}*TB),fps={fps:g}[video];"
        f"[0:a:0]apad=pad_len={pad_samples},"
        f"atrim=end_sample={final_audio_samples}[main_audio]",
        "-map",
        "[video]",
        "-map",
        "[main_audio]",
        *video_codec,
        "-c:a",
        "flac",
        str(destination),
    ]
    subprocess.run(command, check=True)
    result = {
        "schema": "harness.str-conversion/v1",
        "source": str(source),
        "output": str(destination),
        "fps": fps,
        "audio_padding": {
            "primary_stream_file_number": primary_audio["file_number"],
            "primary_stream_channel": primary_audio["channel"],
            "sample_rate": sample_rate,
            "decoded_samples_per_channel": decoded_samples,
            "padding_samples_per_channel": pad_samples,
            "padding_duration_seconds": pad_samples / sample_rate,
            "final_samples_per_channel": final_audio_samples,
            "target_duration_seconds": video_duration,
        },
        "validation": validation,
        "command": command,
    }
    if ffprobe:
        probe = _probe(destination, ffprobe)
        result["ffprobe"] = probe
        result["source_ffprobe"] = source_probe
        packet_ends = _probe_packet_ends(destination, ffprobe)
        video_indices = [
            int(stream["index"])
            for stream in probe.get("streams", [])
            if stream.get("codec_type") == "video"
        ]
        audio_indices = [
            int(stream["index"])
            for stream in probe.get("streams", [])
            if stream.get("codec_type") == "audio"
        ]
        video_end = packet_ends.get(video_indices[0]) if video_indices else None
        audio_end = packet_ends.get(audio_indices[0]) if audio_indices else None
        tolerance = 1.0 / sample_rate
        delta = (
            abs(video_end - audio_end)
            if video_end is not None and audio_end is not None
            else None
        )
        video_stream = next(
            (
                stream
                for stream in probe.get("streams", [])
                if stream.get("codec_type") == "video"
            ),
            {},
        )
        output_frames = int(video_stream.get("nb_read_frames", -1))
        output_rate = Fraction(video_stream.get("avg_frame_rate", "0/1"))
        expected_rate = Fraction(str(fps))
        source_pixel_format = str(source_video.get("pix_fmt", ""))
        output_pixel_format = str(video_stream.get("pix_fmt", ""))
        source_chroma = source_pixel_format.replace("yuvj", "yuv", 1)
        output_chroma = output_pixel_format.replace("yuvj", "yuv", 1)
        shape_preserved = (
            video_stream.get("width") == source_video.get("width")
            and video_stream.get("height") == source_video.get("height")
            and output_chroma == source_chroma
            and video_stream.get("color_range") == source_video.get("color_range")
        )
        timing_pass = (
            delta is not None
            and delta <= tolerance
            and output_frames == int(validation["frame_count"])
            and output_rate == expected_rate
            and shape_preserved
        )
        result["output_timing"] = {
            "video_end_seconds": video_end,
            "audio_end_seconds": audio_end,
            "duration_delta_seconds": delta,
            "source_frame_count": validation["frame_count"],
            "output_frame_count": output_frames,
            "output_frame_rate": str(output_rate),
            "expected_frame_rate": str(expected_rate),
            "source_pixel_format": source_pixel_format,
            "output_pixel_format": output_pixel_format,
            "source_color_range": source_video.get("color_range"),
            "output_color_range": video_stream.get("color_range"),
            "shape_and_chroma_preserved": shape_preserved,
            "tolerance_seconds": tolerance,
            "tolerance_basis": "one audio sample",
            "status": "pass" if timing_pass else "fail",
        }
        result["status"] = "pass" if timing_pass else "fail"
    manifest = output_dir / "conversion.json"
    manifest.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    result["manifest"] = str(manifest)
    return result
