from __future__ import annotations

from ....common import prepend_pythonpath
from ....config import GHIDRA_SRC_DIR


def ghidra_cli_env() -> dict[str, str]:
    return prepend_pythonpath(GHIDRA_SRC_DIR)
