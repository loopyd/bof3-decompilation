from __future__ import annotations

from pathlib import Path

from ..io import ensure_parent, run_command
from ..toolchain.setup_disc import import_bof3_disc
from .inputs import detect_disk_inputs, resolve_disc_input_path


def resolve_project_xml_path(extracted_dir: Path) -> Path | None:
    candidates = sorted(extracted_dir.glob("*.xml"))
    if not candidates:
        return None
    return candidates[0]


def disk_extract(
    *,
    tool_path: Path,
    cwd: Path,
    output_dir: Path,
    disc_dir: Path,
    private_assets_root: Path,
    input_path: Path | None = None,
    archive_path: Path | None = None,
    force: bool = False,
) -> Path:
    if input_path is None:
        if not detect_disk_inputs(disc_dir):
            import_bof3_disc(
                dest=disc_dir,
                archive=archive_path,
                private_assets_root=private_assets_root,
                force=force,
            )
        input_path = resolve_disc_input_path(disc_dir)

    if input_path is None:
        raise RuntimeError(f"no usable disc image found under {disc_dir}")

    output_dir.parent.mkdir(parents=True, exist_ok=True)
    run_command(
        [
            str(tool_path),
            "extract",
            "-i",
            str(input_path),
            "-o",
            str(output_dir),
        ],
        cwd=cwd,
    )
    return input_path


def disk_rebuild(
    *,
    tool_path: Path,
    cwd: Path,
    project_xml_path: Path,
    output_path: Path,
    cue_path: Path,
) -> None:
    ensure_parent(output_path)
    ensure_parent(cue_path)
    run_command(
        [
            str(tool_path),
            "rebuild",
            "-p",
            str(project_xml_path),
            "-o",
            str(output_path),
            "-c",
            str(cue_path),
        ],
        cwd=cwd,
    )


def disk_checksums(
    *,
    tool_path: Path,
    cwd: Path,
    input_dir: Path,
    output_path: Path,
) -> None:
    ensure_parent(output_path)
    run_command(
        [
            str(tool_path),
            "checksum",
            "-i",
            str(input_dir),
            "-o",
            str(output_path),
        ],
        cwd=cwd,
    )


def disk_verify(
    *,
    tool_path: Path,
    cwd: Path,
    input_dir: Path,
    checksums_path: Path,
) -> None:
    run_command(
        [
            str(tool_path),
            "verify",
            "-i",
            str(input_dir),
            "-o",
            str(checksums_path),
        ],
        cwd=cwd,
    )
