from __future__ import annotations

import hashlib
import struct
from pathlib import Path
from unittest.mock import patch

from harness.analysis.engine import EngineIdentity, build_snapshot


def _jal(callsite: int, target: int) -> bytes:
    word = 0x0C000000 | ((target >> 2) & 0x03FFFFFF)
    assert ((callsite + 4) & 0xF0000000) == (target & 0xF0000000)
    return struct.pack("<I", word)


def test_snapshot_records_missing_battle15_static_jals_once(tmp_path: Path) -> None:
    function = 0x800A8CC8
    callsite = 0x800A8CE4
    binary = tmp_path / "battle15.bin"
    binary.write_bytes(
        b"\0" * (callsite - function)
        + _jal(callsite, 0x8014D6B8)
        + _jal(callsite + 4, 0x801DEE4C)
        + b"\0" * 0x50
    )
    engine = EngineIdentity("rizin", tmp_path / "rizin", "test", {})
    functions = [{"offset": function, "size": 0x74, "name": "func_800A8CC8"}]
    xrefs = [
        {"from": callsite, "to": 0x8014D6B8, "type": "CALL"},
        {"from": callsite, "to": 0x8014D6B8, "type": "CALL"},
    ]

    with patch(
        "harness.analysis.engine._run_analysis", return_value=(functions, xrefs)
    ):
        snapshot = build_snapshot(engine, binary, function, "emi/battle/battle/15")

    assert [call.to_row() for call in snapshot.unresolved_calls] == [
        {
            "caller": "emi/battle/battle/15@800a8cc8",
            "target_address": 0x8014D6B8,
            "callsite": callsite,
            "kind": "unknown",
        },
        {
            "caller": "emi/battle/battle/15@800a8cc8",
            "target_address": 0x801DEE4C,
            "callsite": callsite + 4,
            "kind": "static_jal",
        },
    ]


def test_snapshot_psx_payload_hash_and_jal_use_header_offset(tmp_path: Path) -> None:
    function = 0x80100000
    callsite = function + 4
    payload = b"\x08\x00\xe0\x03" + _jal(callsite, 0x80123450) + b"\0" * 8
    binary = tmp_path / "target.exe"
    binary.write_bytes(b"H" * 0x800 + payload)
    engine = EngineIdentity("rizin", tmp_path / "rizin", "test", {})
    functions = [{"offset": function, "size": len(payload), "name": "func_80100000"}]

    with patch("harness.analysis.engine._run_analysis", return_value=(functions, [])):
        snapshot = build_snapshot(
            engine,
            binary,
            function,
            "exe/test",
            binary_offset=0x800,
        )

    assert snapshot.functions[0].exact_sha256 == hashlib.sha256(payload).hexdigest()
    assert snapshot.unresolved_calls[0].callsite == callsite
    assert snapshot.unresolved_calls[0].target_address == 0x80123450


def test_snapshot_ignores_incomplete_final_instruction(tmp_path: Path) -> None:
    binary = tmp_path / "target.bin"
    binary.write_bytes(_jal(0x80000000, 0x80123450) + b"\0\0")
    engine = EngineIdentity("rizin", tmp_path / "rizin", "test", {})
    functions = [{"offset": 0x80000000, "size": 6, "name": "func_80000000"}]

    with patch("harness.analysis.engine._run_analysis", return_value=(functions, [])):
        snapshot = build_snapshot(engine, binary, 0x80000000, "test")

    assert len(snapshot.unresolved_calls) == 1
