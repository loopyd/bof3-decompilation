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

set(BOF3_OBJFLAGS_emi_etc_game_01_func_801D0D5C_c -O1)

# func_801D104C keeps &GAME_FRONT_INPUT_GATE in a base register and reaches the
# nearby front-state fields as negative displacements (lhu/sh -0xF3/-0x23/-0x13).
# The canonical second CSE pass (-frerun-cse-after-loop, implied by -O2) folds
# `front_gate - 0xF3` (front_gate is the fixed address &GAME_FRONT_INPUT_GATE)
# into a fresh symbol-relative lui+lhu per access. Disabling that pass keeps the
# access register-relative off the single base, matching the original byte-for-
# byte with no register pinning. Verified by bin/flag-search (100% exact).
set(BOF3_OBJFLAGS_emi_etc_game_01_func_801D104C_c -O2 -fno-rerun-cse-after-loop)
