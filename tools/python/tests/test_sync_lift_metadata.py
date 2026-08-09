from pathlib import Path

from harness.commands.sync_lift_metadata import sync_lift_metadata
from harness.domain.tags import parse_progress_tags


def row(source: str, status: str, match: float | None, reason: str = "") -> dict:
    return {
        "address": "0x80100000",
        "current_size": 12 if match is not None else None,
        "instruction_count": {"matching": 2, "original": 3} if match is not None else None,
        "match_percent": match,
        "original_size": 16 if match is not None else None,
        "reason": reason,
        "source": source,
        "status": status,
        "target": "test/00",
    }


def test_sync_lift_metadata_writes_exact_partial_and_invalid(tmp_path: Path) -> None:
    rows = [
        row("exact.c", "exact", 100.0),
        row("partial.c", "partial", 66.67),
        row("invalid.c", "invalid", None, "missing boundary"),
    ]
    for item in rows:
        (tmp_path / item["source"]).write_text(
            "/* @behavior test\n * @source 0x80100000\n */\nvoid test(void) {}\n",
            encoding="utf-8",
        )

    assert sync_lift_metadata(tmp_path, {"targets": [{"functions": rows}]}) == 3
    assert parse_progress_tags((tmp_path / "exact.c").read_text()) == (
        "exact",
        100.0,
        "none; live audit is instruction- and byte-exact.",
    )
    assert parse_progress_tags((tmp_path / "partial.c").read_text()) == (
        "partial",
        66.67,
        "non-exact live audit: 2/3 instructions; 16 original bytes versus 12 current.",
    )
    assert parse_progress_tags((tmp_path / "invalid.c").read_text()) == (
        "invalid",
        None,
        "invalid live audit: missing boundary",
    )


def test_sync_lift_metadata_preserves_current_detailed_residual(tmp_path: Path) -> None:
    source = tmp_path / "partial.c"
    source.write_text(
        "/* @behavior test\n * @source 0x80100000\n * @status partial\n"
        " * @match 66.67\n * @residual detailed evidence\n */\nvoid test(void) {}\n",
        encoding="utf-8",
    )

    assert sync_lift_metadata(
        tmp_path, {"targets": [{"functions": [row("partial.c", "partial", 66.67)]}]}
    ) == 0
    assert "detailed evidence" in source.read_text(encoding="utf-8")
