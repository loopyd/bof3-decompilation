"""Pin CUE/BINARY media and stage an isolated descriptor with declared tracks."""

from __future__ import annotations

import re
from pathlib import Path

from harness.io import file_sha256


def collect_disc(path: Path | None) -> tuple[dict, str]:
    if path is None:
        return {}, ""
    path = path.resolve(strict=True)
    if (
        path.suffix.lower() != ".cue"
        or not path.is_file()
        or path.stat().st_size > 1048576
    ):
        raise ValueError("runtime disc requires a CUE file at most 1 MiB")
    data = path.read_bytes()
    entries = {
        "disc:cue": {"path": str(path), "bytes": len(data), "sha256": file_sha256(path)}
    }
    lines = []
    total = 0
    for line in data.decode("utf-8-sig").splitlines():
        if re.match(r"\s*CDTEXTFILE\b", line, re.IGNORECASE):
            raise ValueError("runtime disc does not support external CD-TEXT files")
        if re.match(r"\s*FILE\b", line, re.IGNORECASE):
            match = re.fullmatch(
                r'\s*FILE\s+"([^"\r\n]+)"\s+BINARY\s*', line, re.IGNORECASE
            )
            if not match or len(entries) > 99:
                raise ValueError(
                    "runtime disc supports at most 99 quoted BINARY FILE entries"
                )
            track = (path.parent / match[1]).resolve(strict=True)
            if not track.is_file():
                raise ValueError(f"disc track must be a regular file: {track}")
            size = track.stat().st_size
            total += size
            if not 0 < size <= 1024**3 or total > 2 * 1024**3:
                raise ValueError("runtime disc exceeds 1 GiB per track or 2 GiB total")
            name = f"track{len(entries):02d}.bin"
            entries[f"disc:{name}"] = {
                "path": str(track),
                "bytes": size,
                "sha256": file_sha256(track),
            }
            lines.append(f'FILE "{name}" BINARY')
        else:
            lines.append(line)
    if len(entries) == 1:
        raise ValueError("runtime disc CUE has no BINARY tracks")
    return entries, "\n".join(lines) + "\n"


def stage_disc(output: Path, entries: dict, cue: str) -> Path | None:
    if not entries:
        return None
    folder = output / "disc"
    folder.mkdir()
    for key, entry in entries.items():
        if key != "disc:cue":
            (folder / key.removeprefix("disc:")).symlink_to(entry["path"])
    path = folder / "media.cue"
    path.write_text(cue, encoding="utf-8")
    return path


def encode_bindings(entries: dict) -> str:
    """Expose literal staged-track identities to Lua without executable data."""
    return "".join(
        f"{key}\t{entry['bytes']}\t{entry['sha256']}\n"
        for key, entry in entries.items()
        if key != "disc:cue"
    )


def verify_disc(output: Path, entries: dict, cue: str) -> None:
    if not entries:
        return
    folder = output / "disc"
    expected = {
        "media.cue",
        *(key.removeprefix("disc:") for key in entries if key != "disc:cue"),
    }
    if folder.is_symlink() or (folder / "media.cue").is_symlink():
        raise ValueError("runtime staged disc changed")
    if {p.name for p in folder.iterdir()} != expected or (
        folder / "media.cue"
    ).read_text() != cue:
        raise ValueError("runtime staged disc changed")
    for key, entry in entries.items():
        if key != "disc:cue" and (folder / key.removeprefix("disc:")).resolve(
            strict=True
        ) != Path(entry["path"]):
            raise ValueError("runtime staged disc track changed")
