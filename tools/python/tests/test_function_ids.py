from __future__ import annotations

import pytest

from harness.domain import FUNCTION_ID_FORMAT, FUNCTION_ID_HELP, parse_function_id


def test_parse_function_id_accepts_executable_and_shipped_emi_selectors() -> None:
    executable = parse_function_id("SLUS_004.22@0x8014AE08")
    emi = parse_function_id("BIN/BATTLE/BATL_END.EMI#0@800AF66C")

    assert str(executable) == "exe/slus_004_22@8014ae08"
    assert str(emi) == "emi/battle/batl_end/00@800af66c"


def test_parse_function_id_rejects_incomplete_selector_with_shared_help() -> None:
    with pytest.raises(ValueError, match=FUNCTION_ID_FORMAT):
        parse_function_id("BIN/BATTLE/BATL_END.EMI#0")

    assert "#INDEX@0xADDRESS" in FUNCTION_ID_HELP
