from __future__ import annotations

from ...disk import disk_extract
from ..models import SetupContext


def run(context: SetupContext) -> None:
    disk_extract(
        tool_path=context.layout.bof3_disk_bin,
        cwd=context.layout.root,
        output_dir=context.layout.extracted_dir,
        disc_dir=context.layout.disc_dir,
        private_assets_root=context.layout.private_assets_dir,
        archive_path=context.options.disc_archive,
        force=context.options.force,
    )
