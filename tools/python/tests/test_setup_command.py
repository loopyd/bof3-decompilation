from __future__ import annotations

import argparse
from pathlib import Path

from rebof3.commands.setup import build_setup_options


def test_build_setup_options_open_flag_skips_local_steps() -> None:
    args = argparse.Namespace(
        force=False,
        skip_aspsx_binaries=False,
        skip_match_tools=False,
        skip_psyq=False,
        skip_extract=False,
        skip_ghidra_plan=False,
        psyq_source_root=None,
        psyq_archive=None,
        open_setup=True,
    )

    options = build_setup_options(args)

    assert options.include_psyq is False
    assert options.include_extract is False
    assert options.include_ghidra_plan is False


def test_build_setup_options_preserves_explicit_inputs() -> None:
    args = argparse.Namespace(
        force=True,
        skip_aspsx_binaries=False,
        skip_match_tools=True,
        skip_psyq=False,
        skip_extract=False,
        skip_ghidra_plan=False,
        psyq_source_root=Path("/tmp/psyq"),
        psyq_archive=None,
        open_setup=False,
    )

    options = build_setup_options(args)

    assert options.force is True
    assert options.include_match_tools is False
    assert options.psyq_source_root == Path("/tmp/psyq")
