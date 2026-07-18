# compat/capcom97 — default compiler profile for all sources
#
# Capcom PS1 (BOF3) PsyQ-GCC 2.7.2 compatibility.
# All sources inherit these flags unless overridden.
#
# Evidence: decomp.me PS1 presets, existing BOF3 lift matches.

CPPFLAGS_base   := -DHARNESS_TARGET_PSX=1
CFLAGS_base     := -O2 -G0 -funsigned-char -msoft-float -gcoff
ASPSX_VERSION   := 2.56
CC_ASFLAGS_base := -Wa,--aspsx-version=$(ASPSX_VERSION) -Wa,-G0,-EL,-mips1
ASFLAGS_base    := -G0 -EL -mips1
