from __future__ import annotations

import argparse
import sys
from pathlib import Path

from ..disk import (
    disk_checksums,
    disk_extract,
    disk_rebuild,
    disk_verify,
    resolve_project_xml_path,
)
from ..paths import repo_layout
from ._common import run_main


def _resolve_rebuild_inputs(args: argparse.Namespace) -> tuple[Path, Path, Path]:
    project_xml_path = args.project_xml
    if project_xml_path is None:
        project_xml_path = resolve_project_xml_path(args.extracted_dir)
    if project_xml_path is None:
        raise RuntimeError(f"no project XML found under {args.extracted_dir}")

    output_path = args.output
    if output_path is None:
        output_path = args.rebuilt_dir / f"{project_xml_path.stem}_track01.bin"

    cue_path = args.cue
    if cue_path is None:
        cue_path = output_path.with_suffix(".cue")

    return project_xml_path, output_path, cue_path


def run_extract(args: argparse.Namespace) -> int:
    input_path = disk_extract(
        tool_path=args.tool,
        cwd=args.cwd,
        output_dir=args.output,
        disc_dir=args.disc_dir,
        private_assets_root=args.private_assets_root,
        input_path=args.input,
        archive_path=args.archive,
        force=args.force,
    )
    print(f"extracted {input_path} to {args.output}")
    return 0


def run_rebuild(args: argparse.Namespace) -> int:
    project_xml_path, output_path, cue_path = _resolve_rebuild_inputs(args)
    disk_rebuild(
        tool_path=args.tool,
        cwd=args.cwd,
        project_xml_path=project_xml_path,
        output_path=output_path,
        cue_path=cue_path,
    )
    print(f"rebuilt disc image to {output_path}")
    return 0


def run_verify(args: argparse.Namespace) -> int:
    disk_verify(
        tool_path=args.tool,
        cwd=args.cwd,
        input_dir=args.input_dir,
        checksums_path=args.checksums,
    )
    print(f"verified disk images under {args.input_dir}")
    return 0


def run_checksums(args: argparse.Namespace) -> int:
    disk_checksums(
        tool_path=args.tool,
        cwd=args.cwd,
        input_dir=args.input_dir,
        output_path=args.output,
    )
    print(f"wrote disk checksums to {args.output}")
    return 0


def configure_extract_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--input", type=Path, default=None)
    parser.add_argument("--archive", type=Path, default=None)
    parser.add_argument("--output", type=Path, default=layout.extracted_dir)
    parser.add_argument("--disc-dir", type=Path, default=layout.disc_dir)
    parser.add_argument(
        "--private-assets-root",
        type=Path,
        default=layout.private_assets_dir,
    )
    parser.add_argument("--tool", type=Path, default=layout.harness_disk_bin)
    parser.add_argument("--cwd", type=Path, default=layout.root)
    parser.add_argument("--force", action="store_true")
    parser.set_defaults(handler=run_extract)


def configure_rebuild_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--project-xml", type=Path, default=None)
    parser.add_argument("--extracted-dir", type=Path, default=layout.extracted_dir)
    parser.add_argument("--rebuilt-dir", type=Path, default=layout.rebuilt_dir)
    parser.add_argument("--output", type=Path, default=None)
    parser.add_argument("--cue", type=Path, default=None)
    parser.add_argument("--tool", type=Path, default=layout.harness_disk_bin)
    parser.add_argument("--cwd", type=Path, default=layout.root)
    parser.set_defaults(handler=run_rebuild)


def configure_verify_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--input-dir", type=Path, default=layout.disc_dir)
    parser.add_argument("--checksums", type=Path, default=layout.disk_checksums_path)
    parser.add_argument("--tool", type=Path, default=layout.harness_disk_bin)
    parser.add_argument("--cwd", type=Path, default=layout.root)
    parser.set_defaults(handler=run_verify)


def configure_checksums_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--input-dir", type=Path, default=layout.disc_dir)
    parser.add_argument("--output", type=Path, default=layout.disk_checksums_path)
    parser.add_argument("--tool", type=Path, default=layout.harness_disk_bin)
    parser.add_argument("--cwd", type=Path, default=layout.root)
    parser.set_defaults(handler=run_checksums)


def build_parser(command_name: str) -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog=command_name)
    if command_name == "disk-extract":
        configure_extract_parser(parser)
        return parser
    if command_name == "disk-rebuild":
        configure_rebuild_parser(parser)
        return parser
    if command_name == "disk-verify":
        configure_verify_parser(parser)
        return parser
    if command_name == "disk-checksums":
        configure_checksums_parser(parser)
        return parser
    raise ValueError(f"unsupported disk command: {command_name}")


def main(argv: list[str] | None = None) -> int:
    if not argv:
        raise RuntimeError("missing disk command name")

    command_name, *command_argv = argv
    return run_main(lambda: build_parser(command_name), command_argv)


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
