from __future__ import annotations

import json
from pathlib import Path

from rebof3.paths import repo_layout
from rebof3.source_status import (
    analyze_all_ghidra_function_statuses,
    analyze_ghidra_programs,
    analyze_source_status,
    render_complex_table,
    render_function_table,
    render_ghidra_program_table,
    render_module_table,
    top_complex_functions,
)


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def write_summary(root: Path, function: str, *, exact: bool, match: float) -> None:
    summary_dir = root / function
    summary_dir.mkdir(parents=True)
    (summary_dir / "summary.json").write_text(
        json.dumps(
            {
                "function": function,
                "exact_match": exact,
                "original_size": 16,
                "current_size": 16,
                "size_delta": 0,
                "instruction_count": {"match_percent": match},
            }
        ),
        encoding="utf-8",
    )


def write_ghidra_index(path: Path) -> None:
    write_text(
        path,
        "\t".join(
            [
                "program_path",
                "program_name",
                "program_slug",
                "entry",
                "entry_hex",
                "name",
                "signature",
                "body_min",
                "body_max",
                "namespace",
                "name_source",
                "is_thunk",
                "source_hint",
            ]
        )
        + "\n"
        + "\t".join(
            [
                "/bins/BATTLE/BATTLE/3.bin",
                "3.bin",
                "battle_3",
                "801dc044",
                "0x801dc044",
                "FUN_801dc044",
                "undefined FUN_801dc044(void)",
                "801dc044",
                "801dc73b",
                "Global",
                "DEFAULT",
                "",
                "output/extracted/BATTLE/BATTLE.EMI#3",
            ]
        )
        + "\n"
        + "\t".join(
            [
                "/bins/BATTLE/BATTLE/3.bin",
                "3.bin",
                "battle_3",
                "801dc73c",
                "0x801dc73c",
                "FUN_801dc73c",
                "undefined FUN_801dc73c(void)",
                "801dc73c",
                "801dc893",
                "Global",
                "DEFAULT",
                "",
                "output/extracted/BATTLE/BATTLE.EMI#3",
            ]
        )
        + "\n",
    )


def test_analyze_source_status_summarizes_modules_and_asm(tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)
    write_text(
        layout.bof3_dir / "src" / "core" / "emi" / "internal.h",
        """
typedef struct EmiState {
  u8 phase;
} EmiState;
#define BOF3_EMI_PHASE (*(volatile u8*)0x8014648au)
""",
    )
    write_text(
        layout.bof3_dir / "src" / "core" / "emi" / "func_80162178.c",
        """
#include "internal.h"
extern u32 DAT_80146808;
/* @source: 0x80162178 FUN_80162178 */
u32 func_80162178(void) {
  if (DAT_80146808 != 0u) {
    return DAT_80146808;
  }
  return 0;
}
""",
    )
    asm_root = tmp_path / "asm-diff"
    write_summary(asm_root, "func_80162178", exact=True, match=100.0)

    modules = analyze_source_status(layout, asm_root=asm_root)

    assert len(modules) == 1
    module = modules[0]
    assert module.module == "core/emi"
    assert module.functions == 1
    assert module.source_tagged == 1
    assert module.asm_summaries == 1
    assert module.exact_matches == 1
    assert module.avg_match_percent == 100.0
    assert "BOF3_EMI_PHASE" in module.variables
    assert "DAT_80146808" in module.variables
    assert "EmiState" in module.structs
    assert module.ghidra_functions == 0


def test_render_tables_include_complex_candidates(tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)
    write_text(
        layout.bof3_dir / "src" / "modules" / "battle" / "03" / "func_801dc044.c",
        """
/* @source: 0x801dc044 FUN_801dc044 */
u32 func_801dc044(u8 arg0) {
  if (arg0) {
    while (arg0 > 1) {
      arg0 -= 1;
    }
  }
  return arg0;
}
""",
    )

    modules = analyze_source_status(layout, asm_root=tmp_path / "missing")
    candidates = top_complex_functions(modules, limit=1)

    assert candidates[0].name == "func_801dc044"
    assert "modules/battle/03" in render_module_table(modules)
    assert "func_801dc044" in render_complex_table(candidates)


def test_analyze_source_status_includes_ghidra_functions(tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)
    write_text(
        layout.bof3_dir / "src" / "modules" / "battle" / "03" / "func_801dc044.c",
        """
/* @source: 0x801dc044 FUN_801dc044 */
u32 func_801dc044(u8 arg0) { return arg0; }
""",
    )
    ghidra_index = tmp_path / "ghidra_function_index.tsv"
    write_ghidra_index(ghidra_index)

    modules = analyze_source_status(
        layout,
        ghidra_function_index_tsv=ghidra_index,
    )
    programs = analyze_ghidra_programs(
        layout,
        module_filter="modules/battle/03",
        ghidra_function_index_tsv=ghidra_index,
    )

    module = modules[0]
    assert module.ghidra_functions == 2
    assert module.ghidra_lifted == 1
    assert module.ghidra_unlifted == 1
    assert module.ghidra_unlifted_samples == ["0x801dc73c:FUN_801dc73c"]
    assert [status.status for status in module.merged_function_statuses] == [
        "lifted",
        "unlifted",
    ]
    assert module.merged_function_statuses[0].source_name == "func_801dc044"
    assert module.merged_function_statuses[1].ghidra_name == "FUN_801dc73c"
    assert programs[0].ghidra_functions == 2
    assert programs[0].lifted_functions == 1
    assert "FUN_801dc73c" in render_ghidra_program_table(programs)
    function_table = render_function_table(modules)
    assert "0x801dc044\tlifted\tFUN_801dc044\tfunc_801dc044" in function_table
    assert "0x801dc73c\tunlifted\tFUN_801dc73c" in function_table


def test_all_ghidra_function_statuses_include_bin_only_rows(tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)
    write_text(
        layout.bof3_dir / "src" / "modules" / "battle" / "03" / "func_801dc044.c",
        """
/* @source: 0x801dc044 FUN_801dc044 */
u32 func_801dc044(u8 arg0) { return arg0; }
""",
    )
    ghidra_index = tmp_path / "ghidra_function_index.tsv"
    write_ghidra_index(ghidra_index)
    modules = analyze_source_status(
        layout,
        ghidra_function_index_tsv=ghidra_index,
    )

    rows = analyze_all_ghidra_function_statuses(
        layout,
        modules,
        ghidra_function_index_tsv=ghidra_index,
    )

    assert [row.status for row in rows] == ["lifted", "unlifted"]
    assert rows[0].module == "modules/battle/03"
    assert rows[1].module == "modules/battle/03"
