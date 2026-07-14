# Semantic aliases are additional flags, never replacements for address-based
# function names. These aliases are reviewed in the owning symbol layer.
fs semantic
f semantic.emi_loader_initialize 1 @ 0x80161f58
f semantic.emi_stream_init_slot 1 @ 0x80161fdc
f semantic.emi_loader_slot_lba 1 @ 0x80162160
f semantic.emi_cd_sync_callback 1 @ 0x801621e8
f semantic.emi_cd_ready_callback 1 @ 0x80162230
f semantic.emi_loader_is_ready 1 @ 0x80162d00

# PsyQ 4.7 SDK call documentation — flags auto-generated from symbols/psyq.c;
# these CC comments describe each function for interactive browsing.
CC "CdSync(mode, result) — wait for CD operation" @ 0x80175640
CC "CdReady(cb, result) — check if CD is ready" @ 0x80175660
CC "CdSearchFile(fp, name) — find file on disc" @ 0x80177348
CC "VSync(0..3) — wait N vblanks; returns vblank count" @ 0x80174700
CC "ClearOTagR(ot, n) — clear reverse ordering table" @ 0x8017b8d4
CC "DrawOTag(ot) — submit ordering table to GPU" @ 0x8017b9cc
CC "PutDrawEnv(env) — apply drawing environment" @ 0x8017ba40
CC "PutDispEnv(env) — apply display environment" @ 0x8017bc98
CC "DrawSync(mode) — wait for GPU to finish" @ 0x8017b3cc
CC "OpenEvent(desc, spec, mode, func) — desc=SwCARD|HwCARD" @ 0x8017ed3c
CC "EnableEvent(event) — enable event for delivery" @ 0x8017ed7c

# PsyQ 4.7 kernel descriptor/event constants (kernel.h).
fs constants
f const.DescHW 4 @ 0x8017ed3c
CC "DescHW=0xf0000000 DescSW=0xf4000000 EvMdINTR=0x1000 EvMdNOINTR=0x2000" @ 0x8017ed3c

# Boot/data area flags — addressable globals referenced by reviewed functions.
fs data
f data.DAT_80143e68 4 @ 0x80143e68
CC "display work pointer; double-buffered (2x 0x90 bytes)" @ 0x80143e68
f data.DAT_80143d44 1 @ 0x80143d44
CC "frame toggle index (0/1 xor)" @ 0x80143d44
f data.DAT_80143d48 4 @ 0x80143d48
CC "work buffer base[2]" @ 0x80143d48
f data.DAT_80145aa4 2 @ 0x80145aa4
CC "gfx state flags" @ 0x80145aa4
f data.DAT_80143f44 1 @ 0x80143f44
CC "EMI-service guarded frame counter" @ 0x80143f44
f data.DAT_80143ef8 4 @ 0x80143ef8
CC "last VSync return value" @ 0x80143ef8
f data.DAT_80143e6c 4 @ 0x80143e6c
CC "absolute frame counter" @ 0x80143e6c
f data.DAT_8018b300 4 @ 0x8018b300
CC "boot-level init flag" @ 0x8018b300

# Event handles from func_8014b1a4 event setup.
f data.DAT_80145e14 4 @ 0x80145e14
f data.DAT_80145e18 4 @ 0x80145e18
f data.DAT_80145e1c 4 @ 0x80145e1c
f data.DAT_80145e20 4 @ 0x80145e20
f data.DAT_80145e24 4 @ 0x80145e24
f data.DAT_80145e28 4 @ 0x80145e28
f data.DAT_80145e2c 4 @ 0x80145e2c
CC "SwCARD/HwCARD event handles (func_8014b1a4)" @ 0x80145e14

fs functions
