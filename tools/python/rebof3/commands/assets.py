from __future__ import annotations

import argparse
import sys
from pathlib import Path

from ..assets import (
    build_review_packet,
    extract_archive,
    extract_tree,
    preview_indexed_image,
    render_status_archive,
    render_title_bundle,
)
from ..emi import emi_unpack
from ..inventory.build import write_markdown
from ..inventory.emi_catalog import (
    build_emi_manifest_catalog,
    render_emi_catalog_markdown,
)
from ..inventory.render_metadata import (
    build_render_metadata,
    render_render_metadata_markdown,
)
from ..jsonio import write_json
from ..paths import repo_layout
from ._common import run_main


def assets_output_root() -> Path:
    return repo_layout().out_dir / "emi_assets"


def parse_int(text: str) -> int:
    return int(text, 0)


def run_extract(args: argparse.Namespace) -> int:
    archive_count = emi_unpack(
        tool_path=args.tool,
        cwd=args.cwd,
        extracted_dir=args.input_dir,
        raw_emi_dir=args.output_dir,
    )
    print(f"unpacked {archive_count} EMI archives into {args.output_dir}")

    if args.skip_catalog and args.skip_render_metadata:
        return 0

    emi_root = args.emi_root if args.emi_root is not None else args.output_dir / "BIN"
    catalog = build_emi_manifest_catalog(emi_root)
    if not args.skip_catalog:
        write_json(args.emi_catalog_json_out, catalog)
        write_markdown(args.emi_catalog_md_out, render_emi_catalog_markdown(catalog))
        print(f"wrote EMI catalog to {args.emi_catalog_json_out}")
    if not args.skip_render_metadata:
        metadata = build_render_metadata(catalog)
        write_json(args.render_metadata_json_out, metadata)
        write_markdown(
            args.render_metadata_md_out,
            render_render_metadata_markdown(metadata),
        )
        print(f"wrote render metadata to {args.render_metadata_json_out}")
    return 0


def run_review(args: argparse.Namespace) -> int:
    result = build_review_packet(
        catalog_path=args.catalog,
        output_root=args.output_root,
        families=args.family,
        archive_substrings=args.archive_substr,
        clean=args.clean,
        emit_indices=args.emit_indices,
    )
    print(f"wrote review manifest to {result['manifest_path']}")
    print(f"selected {len(result['selected_archives'])} archives")
    return 0


def run_extract_archive(args: argparse.Namespace) -> int:
    written = extract_archive(
        args.archive,
        args.output_dir,
        image_selectors=args.image,
        palette_selectors=args.palette,
        bpp_override=args.bpp,
        width_override=args.width,
        palette_row=args.palette_row,
        columns=args.columns,
        unstrip=not args.no_unstrip,
        emit_indices=args.emit_indices,
        emit_palette_previews=args.emit_palette_previews,
    )
    print(f"wrote {len(written)} files to {args.output_dir}")
    return 0


def run_extract_tree(args: argparse.Namespace) -> int:
    result = extract_tree(
        args.root,
        args.output_dir,
        bpp_override=args.bpp,
        palette_row=args.palette_row,
        columns=args.columns,
        unstrip=not args.no_unstrip,
        emit_indices=args.emit_indices,
    )
    print(
        f"processed {result['archive_count']} archives, {result['image_count']} images, "
        f"{result['written_count']} outputs"
    )
    return 0


def run_render_title(args: argparse.Namespace) -> int:
    first_path = args.first
    demo_path = args.demo
    game_path = args.game
    if args.etc_root is not None:
        first_path = args.etc_root / "FIRST.EMI"
        demo_path = args.etc_root / "DEMO.EMI"
        game_path = args.etc_root / "GAME.EMI"

    render_title_bundle(
        first_path=first_path,
        demo_path=demo_path,
        game_path=game_path,
        output_dir=args.output_dir,
        clean=args.clean,
    )
    print(f"rendered title ETC assets to {args.output_dir}")
    return 0


def run_render_status(args: argparse.Namespace) -> int:
    render_status_archive(
        archive_path=args.archive,
        output_dir=args.output_dir,
        game_archive_path=args.game_archive,
        clean=args.clean,
    )
    print(f"rendered STATUS assets to {args.output_dir}")
    return 0


def run_preview(args: argparse.Namespace) -> int:
    preview_indexed_image(
        image_path=args.image,
        palette_path=args.palette,
        output_path=args.output,
        bpp=args.bpp,
        stripped_width=args.width,
        palette_row=args.palette_row,
        contact_sheet=args.contact_sheet,
        columns=args.columns,
        unstrip=not args.no_unstrip,
    )
    print(f"wrote preview to {args.output}")
    return 0


def configure_extract_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--input-dir", type=Path, default=layout.extracted_dir)
    parser.add_argument("--output-dir", type=Path, default=layout.raw_emi_dir)
    parser.add_argument("--emi-root", type=Path, default=None)
    parser.add_argument("--tool", type=Path, default=layout.emi_ex_bin)
    parser.add_argument("--cwd", type=Path, default=layout.root)
    parser.add_argument(
        "--emi-catalog-json-out", type=Path, default=layout.inventory_emi_catalog_path
    )
    parser.add_argument(
        "--emi-catalog-md-out", type=Path, default=layout.inventory_emi_catalog_md_path
    )
    parser.add_argument(
        "--render-metadata-json-out",
        type=Path,
        default=layout.inventory_render_metadata_path,
    )
    parser.add_argument(
        "--render-metadata-md-out",
        type=Path,
        default=layout.inventory_render_metadata_md_path,
    )
    parser.add_argument("--skip-catalog", action="store_true")
    parser.add_argument("--skip-render-metadata", action="store_true")
    parser.set_defaults(handler=run_extract)


def configure_review_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument(
        "--catalog", type=Path, default=layout.inventory_emi_catalog_path
    )
    parser.add_argument(
        "--output-root", type=Path, default=assets_output_root() / "review"
    )
    parser.add_argument("--family", action="append", default=None)
    parser.add_argument("--archive-substr", action="append", default=None)
    parser.add_argument("--clean", action="store_true")
    parser.add_argument("--emit-indices", action="store_true")
    parser.set_defaults(handler=run_review)


def configure_extract_archive_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("archive", type=Path)
    parser.add_argument("output_dir", type=Path)
    parser.add_argument("--image", action="append", default=None)
    parser.add_argument("--palette", action="append", default=None)
    parser.add_argument("--bpp", type=int, choices=[4, 8], default=None)
    parser.add_argument("--width", type=int, default=None)
    parser.add_argument("--palette-row", type=int, default=None)
    parser.add_argument("--columns", type=int, default=4)
    parser.add_argument("--no-unstrip", action="store_true")
    parser.add_argument("--emit-indices", action="store_true")
    parser.add_argument("--emit-palette-previews", action="store_true")
    parser.set_defaults(handler=run_extract_archive)


def configure_extract_tree_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--root", type=Path, default=layout.emi_root)
    parser.add_argument(
        "--output-dir", type=Path, default=assets_output_root() / "tree"
    )
    parser.add_argument("--bpp", type=int, choices=[4, 8], default=None)
    parser.add_argument("--palette-row", type=int, default=None)
    parser.add_argument("--columns", type=int, default=4)
    parser.add_argument("--no-unstrip", action="store_true")
    parser.add_argument("--emit-indices", action="store_true")
    parser.set_defaults(handler=run_extract_tree)


def configure_render_title_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--etc-root", type=Path, default=None)
    parser.add_argument(
        "--first", type=Path, default=layout.extracted_dir / "BIN" / "ETC" / "FIRST.EMI"
    )
    parser.add_argument(
        "--demo", type=Path, default=layout.extracted_dir / "BIN" / "ETC" / "DEMO.EMI"
    )
    parser.add_argument(
        "--game", type=Path, default=layout.extracted_dir / "BIN" / "ETC" / "GAME.EMI"
    )
    parser.add_argument(
        "--output-dir", type=Path, default=assets_output_root() / "title"
    )
    parser.add_argument("--clean", action="store_true")
    parser.set_defaults(handler=run_render_title)


def configure_render_status_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument(
        "--archive",
        type=Path,
        default=layout.extracted_dir / "BIN" / "ETC" / "STATUS.EMI",
    )
    parser.add_argument(
        "--game-archive",
        type=Path,
        default=layout.extracted_dir / "BIN" / "ETC" / "GAME.EMI",
    )
    parser.add_argument(
        "--output-dir", type=Path, default=assets_output_root() / "status"
    )
    parser.add_argument("--clean", action="store_true")
    parser.set_defaults(handler=run_render_status)


def configure_preview_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("image", type=Path)
    parser.add_argument("palette", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--bpp", type=int, choices=[4, 8], default=4)
    parser.add_argument("--width", type=int, default=128)
    parser.add_argument("--palette-row", type=int, default=0)
    parser.add_argument("--contact-sheet", action="store_true")
    parser.add_argument("--columns", type=int, default=4)
    parser.add_argument("--no-unstrip", action="store_true")
    parser.set_defaults(handler=run_preview)


def build_parser(command_name: str) -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog=command_name)
    if command_name == "emi-extract":
        configure_extract_parser(parser)
        return parser
    if command_name == "emi-review":
        configure_review_parser(parser)
        return parser
    if command_name == "emi-extract-archive":
        configure_extract_archive_parser(parser)
        return parser
    if command_name == "emi-extract-tree":
        configure_extract_tree_parser(parser)
        return parser
    if command_name == "emi-render-title":
        configure_render_title_parser(parser)
        return parser
    if command_name == "emi-render-status":
        configure_render_status_parser(parser)
        return parser
    if command_name == "emi-preview":
        configure_preview_parser(parser)
        return parser
    raise ValueError(f"unsupported assets command: {command_name}")


def main(argv: list[str] | None = None) -> int:
    if not argv:
        raise RuntimeError("missing assets command name")
    command_name, *command_argv = argv
    return run_main(lambda: build_parser(command_name), command_argv)


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
