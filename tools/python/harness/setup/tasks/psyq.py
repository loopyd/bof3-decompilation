from __future__ import annotations

from ...toolchain.setup_psyq import stage_psyq_sdk
from ..models import SetupContext


def run(context: SetupContext) -> None:
    stage_psyq_sdk(
        dest=context.layout.psyq_root if context.options.psyq_version is None else None,
        source_root=context.options.psyq_source_root,
        archive=context.options.psyq_archive,
        version=context.options.psyq_version,
        force=context.options.force,
    )
