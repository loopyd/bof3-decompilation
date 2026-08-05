# Per-object compiler-flag overrides for BOF3 lifts.
#
# The canonical build compiles every C source with BOF3_CFLAGS
# (-O2 -G0 -funsigned-char -msoft-float -gcoff). Some originals were compiled at
# a different profile; for those, record the verified profile here so the object
# actually builds with it. Sources without an entry keep the canonical flags.
#
# Key: source path relative to src/, sanitized (every non-alphanumeric byte ->
# underscore), prefixed with BOF3_OBJFLAGS_. This mirrors CMake's
# string(MAKE_C_IDENTIFIER ...) and the parser in
# tools/python/harness/commands/compile_commands.py, so the build and the
# compile database stay in sync.
#
# Value: a bin/flag-search candidate that REPLACES the canonical -O level and
# appends scheduling flags (the canonical -G0/-funsigned-char/... base is kept).
#
# Add an entry ONLY after `bin/flag-search TARGET@0xADDR` reports the profile as
# an exact byte-match, then re-confirm with `bin/byte-match TARGET@0xADDR`.
#
# Format:  set(BOF3_OBJFLAGS_<sanitized_src_relative_path> <flags...>)
# Example: set(BOF3_OBJFLAGS_emi_etc_game_01_func_801D0D5C_c -O1)
#
# Per-object compiler variant override:
#   set(BOF3_OBJCOMPILER_<sanitized_src_relative_path> <catalog-id>)
# The <catalog-id> must match an entry in config/compiler/variants.json.
# CMake wraps the compile command in `cmake -E env PSX_GCC=<verified-path>`
# when this is set. No entry means canonical gcc-2.7.2-psx is used.

set(BOF3_OBJFLAGS_emi_etc_game_01_func_801D0D5C_c -O1)

# func_801D104C keeps &GAME_FRONT_INPUT_GATE in a base register and reaches the
# nearby front-state fields as negative displacements (lhu/sh -0xF3/-0x23/-0x13).
# The canonical second CSE pass (-frerun-cse-after-loop, implied by -O2) folds
# `front_gate - 0xF3` (front_gate is the fixed address &GAME_FRONT_INPUT_GATE)
# into a fresh symbol-relative lui+lhu per access. Disabling that pass keeps the
# access register-relative off the single base, matching the original byte-for-
# byte with no register pinning. Verified by bin/flag-search (100% exact).
set(BOF3_OBJFLAGS_emi_etc_game_01_func_801D104C_c -O2 -fno-rerun-cse-after-loop)

# func_801E29B4 uses a signed modulo (rand() % count) whose original expansion
# includes the full MIPS division-trap sequence (break 7 / break 6). The
# canonical maspsx pass omits these traps; --expand-div restores them.
# Verified by bin/flag-search (no -O variant matches) + manual maspsx test.
set(BOF3_OBJFLAGS_emi_battle_battle_03_func_801E29B4_c -O2 -Wa,--expand-div)

# func_800AB760 is a byte-identical duplicate of func_801E29B4 (battle/03).
# Same signed modulo (rand() % count) requiring --expand-div for the MIPS
# division-trap sequence.
set(BOF3_OBJFLAGS_emi_battle_battle_15_func_800AB760_c -O2 -Wa,--expand-div)
set(BOF3_OBJCOMPILER_exe_slus_004_22_func_8015DF18_c gcc-2.6.3-psx)
