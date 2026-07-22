"""BOF3 BGM player: render SEP+VAB from disc-extracted EMI archives."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from ..io import repo_layout

_BGM_DIR = "out/extracted/BIN/BGM"

_TRACK_NAMES: dict[str, str] = {
    "battle 1": "BGMBAT01",
    "battle 2": "BGMBAT02",
    "battle 3": "BGMBAT03",
    "battle 4": "BGMBAT04",
    "battle 5": "BGMBAT05",
    "battle 6": "BGMBAT06",
    "boss": "BGMBAT07",
    "field": "BGM000",
    "town": "BGM002",
    "dungeon": "BGM003",
    "ending": "BGMEND",
}


def _find_bgm(root: Path, query: str) -> Path | None:
    bgm_dir = root / _BGM_DIR
    if not bgm_dir.is_dir():
        return None
    normalized = query.strip().lower().replace("_", " ")
    if normalized in _TRACK_NAMES:
        name = _TRACK_NAMES[normalized]
    else:
        name = query.strip().upper()
        if not name.startswith("BGM"):
            name = f"BGM{name}"
    emi = bgm_dir / f"{name}.EMI"
    extracted = bgm_dir / name
    if extracted.is_dir():
        return extracted
    if emi.is_file():
        return emi
    for candidate in sorted(bgm_dir.iterdir()):
        if name.lower() in candidate.stem.lower():
            return candidate
    return None


def _load_entries(path: Path) -> tuple[bytes, bytes, bytes]:
    if path.is_dir():
        vh = (path / "0.bin").read_bytes()
        sep = (path / "1.bin").read_bytes()
        vb = (path / "2.bin").read_bytes()
    else:
        import struct

        data = path.read_bytes()
        count = struct.unpack_from("<I", data, 0)[0]
        entries: list[tuple[int, int]] = []
        off = 8
        for _ in range(count):
            _name = data[off : off + 16].split(b"\x00")[0]
            _type, _ram, size, data_off = struct.unpack_from("<IIII", data, off + 16)
            entries.append((data_off, size))
            off += 32
        vh = data[entries[0][0] : entries[0][0] + entries[0][1]]
        sep = data[entries[1][0] : entries[1][0] + entries[1][1]]
        vb = data[entries[2][0] : entries[2][0] + entries[2][1]]
    return vh, sep, vb


def run_play(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    bgm_path = _find_bgm(root, args.track)
    if bgm_path is None:
        print(f"error: track not found: {args.track}", file=sys.stderr)
        print(f"  available in {root / _BGM_DIR}/", file=sys.stderr)
        return 1

    from ..assets.audio._renderer import render_bgm
    from ..assets.audio._player import play_pcm
    from ..assets.audio.sep import parse_sep

    vh_data, sep_data, vb_data = _load_entries(bgm_path)
    sep = parse_sep(sep_data)
    seq_idx = args.seq
    if seq_idx >= len(sep.sequences):
        print(
            f"error: seq {seq_idx} not found ({len(sep.sequences)} available)",
            file=sys.stderr,
        )
        return 1
    seq = sep.sequences[seq_idx]
    bpm = 60_000_000 / seq.tempo_us if seq.tempo_us else 0
    name = bgm_path.stem if bgm_path.is_dir() else bgm_path.stem
    print(f"  {name} — seq {seq_idx} ({bpm:.0f} BPM)")
    print("  rendering...")
    result = render_bgm(sep_data, vh_data, vb_data, sequence_index=seq_idx)
    play_pcm(result.samples, result.sample_rate, result.channels, gain=args.gain)
    return 0


def run_list(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    bgm_dir = root / _BGM_DIR
    if not bgm_dir.is_dir():
        print(f"error: no BGM directory at {bgm_dir}", file=sys.stderr)
        print("  extract the disc first: bin/bof3-disk extract", file=sys.stderr)
        return 1
    tracks = sorted(
        {p.stem for p in bgm_dir.iterdir() if p.suffix == ".EMI" or p.is_dir()}
    )
    friendly = {v: k for k, v in _TRACK_NAMES.items()}
    for t in tracks:
        alias = friendly.get(t, "")
        suffix = f'  ("{alias}")' if alias else ""
        print(f"  {t}{suffix}")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bof3-audio", description="BOF3 BGM player")
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    sub = parser.add_subparsers(dest="command")

    play_p = sub.add_parser("play", help="render and play a BGM track")
    play_p.add_argument(
        "track", help='track name or BGM id (e.g. "Battle 2" or BGMBAT04)'
    )
    play_p.add_argument("--seq", type=int, default=0, help="sequence index within SEP")
    play_p.add_argument("--gain", type=float, default=1.0, help="volume gain (0.0-1.0)")
    play_p.set_defaults(handler=run_play)

    list_p = sub.add_parser("list", help="list available BGM tracks")
    list_p.set_defaults(handler=run_list)

    return parser


def main(argv: list[str] | None = None) -> int:
    from ._common import run_main

    arguments = sys.argv[1:] if argv is None else argv
    return run_main(build_parser, arguments)


if __name__ == "__main__":
    raise SystemExit(main())
