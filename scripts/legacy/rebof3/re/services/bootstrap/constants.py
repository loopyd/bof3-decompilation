from __future__ import annotations

from pathlib import Path

from ....config import GHIDRA_MAIN_MODULE, GHIDRA_SRC_DIR, ROOT

DEFAULT_GHIDRA_HOME = Path("/opt/ghidra")
DEFAULT_PROJECT_NAME = "bof3_main"

__all__ = [
    "DEFAULT_GHIDRA_HOME",
    "DEFAULT_PROJECT_NAME",
    "GHIDRA_MAIN_MODULE",
    "GHIDRA_SRC_DIR",
    "ROOT",
]
