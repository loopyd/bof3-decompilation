from __future__ import annotations

import json

from harness.commands.lift import _print_match
from harness.output import resolve_detail


def _payload() -> dict[str, object]:
    return {
        "schema": "harness.asm-diff-one/v2",
        "function": "func_80100010",
        "address": "0x80100010",
        "status": "different",
        "byte_match": False,
        "exact_match": False,
        "instruction_count": {
            "original": 4,
            "current": 5,
            "matching": 3,
            "match_percent": 60.0,
        },
        "original_size": 16,
        "current_size": 20,
        "size_delta": 4,
        "first_mismatch": {"original_offset": 4, "current_offset": 4},
        "outputs": {"diff": "/tmp/diff.patch"},
    }


def test_json_detail_projects_match_payload(capsys) -> None:
    assert (
        _print_match(_payload(), json_output=True, bytes_only=False, detail="minimal")
        == 1
    )

    rendered = json.loads(capsys.readouterr().out)
    assert set(rendered) == {
        "schema",
        "function",
        "address",
        "status",
        "byte_match",
        "instruction_count",
    }


def test_json_without_detail_remains_full() -> None:
    assert resolve_detail(requested=None, json_output=True) == "full"
