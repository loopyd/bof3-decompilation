from __future__ import annotations

from pathlib import Path
from types import SimpleNamespace

import pytest

from harness.commands.symbols import main as symbols_main
from harness.domain.naming_debt import (
    collect_naming_debt,
    load_naming_baseline,
    naming_debt_regressions,
)


def test_naming_debt_reports_new_raw_and_invalid_names(tmp_path: Path) -> None:
    source = tmp_path / "src" / "bof3" / "battle"
    source.mkdir(parents=True)
    (source / "func_80100000.c").write_text("", encoding="utf-8")
    (source / "bad_name.c").write_text("", encoding="utf-8")
    target = tmp_path / "config" / "targets" / "exe" / "game"
    target.mkdir(parents=True)
    (target / "symbols.txt").write_text(
        "D_80100004 = 0x80100004;\nfunc_80100000 = 0x80100000;\n",
        encoding="utf-8",
    )

    debt = collect_naming_debt(
        tmp_path, {"exe/game": SimpleNamespace(id="exe/game")}
    )

    assert debt.raw_function_files == frozenset(
        {"src/bof3/battle/func_80100000.c"}
    )
    assert debt.invalid_semantic_files == frozenset(
        {"src/bof3/battle/bad_name.c"}
    )
    assert debt.raw_functions == frozenset({"exe/game:func_80100000"})
    assert debt.raw_data == frozenset({"exe/game:D_80100004"})
    assert naming_debt_regressions(debt, {}) == [
        "new naming debt (raw_function_files): src/bof3/battle/func_80100000.c",
        "new naming debt (invalid_semantic_files): src/bof3/battle/bad_name.c",
        "new naming debt (raw_functions): exe/game:func_80100000",
        "new naming debt (raw_data): exe/game:D_80100004",
    ]


def test_naming_debt_allows_existing_debt_and_reductions(tmp_path: Path) -> None:
    debt = collect_naming_debt(tmp_path, {})
    baseline = {
        "raw_function_files": {"src/bof3/battle/func_80100000.c"},
        "invalid_semantic_files": set(),
        "raw_functions": {"exe/game:func_80100000"},
        "raw_data": {"exe/game:D_80100004"},
    }

    assert naming_debt_regressions(debt, baseline) == []


def test_naming_debt_only_exempts_top_level_support(tmp_path: Path) -> None:
    nested = tmp_path / "src" / "bof3" / "battle" / "support"
    nested.mkdir(parents=True)
    (nested / "bad_name.c").write_text("", encoding="utf-8")

    debt = collect_naming_debt(tmp_path, {})

    assert debt.invalid_semantic_files == frozenset(
        {"src/bof3/battle/support/bad_name.c"}
    )


def test_missing_naming_baseline_fails_closed(tmp_path: Path) -> None:
    with pytest.raises(ValueError, match="missing naming baseline"):
        load_naming_baseline(tmp_path)


def test_symbols_check_requires_baseline_for_global_check(
    tmp_path: Path, capsys
) -> None:
    (tmp_path / "config" / "targets" / "shared").mkdir(parents=True)
    (tmp_path / "config" / "targets" / "shared" / "symbols.txt").write_text(
        "", encoding="utf-8"
    )

    code = symbols_main(["--root", str(tmp_path), "check"])

    assert code == 2
    assert "missing naming baseline" in capsys.readouterr().err
