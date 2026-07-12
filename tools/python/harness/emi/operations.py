from __future__ import annotations

from pathlib import Path

from ..common import run_command


def _find_emi_archives(extracted_dir: Path) -> list[Path]:
    return sorted(extracted_dir.rglob("*.EMI"))


def _find_unpacked_emi_dirs(raw_emi_dir: Path) -> list[Path]:
    return sorted(path.parent for path in raw_emi_dir.rglob("emi.json"))


def _archive_output_dir(
    *,
    extracted_dir: Path,
    raw_emi_dir: Path,
    archive_path: Path,
) -> Path:
    relative_archive_path = archive_path.relative_to(extracted_dir)
    return raw_emi_dir / relative_archive_path.with_suffix("")


def _packed_archive_path(
    *,
    raw_emi_dir: Path,
    extracted_dir: Path,
    archive_dir: Path,
) -> Path:
    relative_archive_dir = archive_dir.relative_to(raw_emi_dir)
    return extracted_dir / relative_archive_dir.with_suffix(".EMI")


def emi_unpack(
    *,
    tool_path: Path,
    cwd: Path,
    extracted_dir: Path,
    raw_emi_dir: Path,
) -> int:
    archive_paths = _find_emi_archives(extracted_dir)
    if not archive_paths:
        raise RuntimeError(f"no EMI archives found under {extracted_dir}")

    for archive_path in archive_paths:
        output_dir = _archive_output_dir(
            extracted_dir=extracted_dir,
            raw_emi_dir=raw_emi_dir,
            archive_path=archive_path,
        )
        output_dir.parent.mkdir(parents=True, exist_ok=True)
        run_command(
            [
                str(tool_path),
                "extract",
                "--quiet",
                "-J",
                "-o",
                str(output_dir),
                str(archive_path),
            ],
            cwd=cwd,
        )

    return len(archive_paths)


def emi_pack(
    *,
    tool_path: Path,
    cwd: Path,
    raw_emi_dir: Path,
    extracted_dir: Path,
) -> int:
    archive_dirs = _find_unpacked_emi_dirs(raw_emi_dir)
    if not archive_dirs:
        raise RuntimeError(f"no unpacked EMI manifests found under {raw_emi_dir}")

    for archive_dir in archive_dirs:
        output_path = _packed_archive_path(
            raw_emi_dir=raw_emi_dir,
            extracted_dir=extracted_dir,
            archive_dir=archive_dir,
        )
        output_path.parent.mkdir(parents=True, exist_ok=True)
        run_command(
            [
                str(tool_path),
                "pack",
                "--quiet",
                "-o",
                str(output_path),
                "-J",
                str(archive_dir / "emi.json"),
                str(archive_dir),
            ],
            cwd=cwd,
        )

    return len(archive_dirs)
