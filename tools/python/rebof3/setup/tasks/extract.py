from __future__ import annotations

from ...common import run_command
from ...toolchain.setup_disc import import_bof3_disc
from ..models import SetupContext
from .helpers import detect_disk_inputs, resolve_disc_input_path


def run(context: SetupContext) -> None:
    if not detect_disk_inputs(context):
        import_bof3_disc(
            dest=context.layout.disc_dir,
            archive=context.options.disc_archive,
            private_assets_root=context.layout.private_assets_dir,
            force=context.options.force,
        )
    if not detect_disk_inputs(context):
        raise RuntimeError(f"no disc image found under {context.layout.disc_dir}")
    disc_input_path = resolve_disc_input_path(context)
    if disc_input_path is None:
        raise RuntimeError(
            f"no usable disc image found under {context.layout.disc_dir}"
        )
    run_command(
        [
            str(context.layout.bof3_disk_bin),
            "extract",
            "-i",
            str(disc_input_path),
            "-o",
            str(context.layout.extracted_dir),
        ],
        cwd=context.layout.root,
    )
