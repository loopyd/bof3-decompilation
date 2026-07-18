---
type: Runtime
title: Source retention audit
description: Retained and removed lifted C sources after canonical map/link migration.
---

# Source retention audit

The 2026 canonical migration retained 299 tracked C sources: 88 byte-exact and 211 partial but valid linked diffs. It removed the 119 sources below because they could not produce a valid target-qualified diff. No archive tree is retained; Git history is the recovery record.

| Source | Target | Address | Removal reason |
| --- | --- | --- | --- |
| `src/emi/battle/batl_re2/01/func_80036E00.c` | `emi/battle/batl_re2/01` | `0x80036E00` | cannot infer original size for 0x80036e00 from /home/rcorreia/projects/rebof3-simple/out/binaries/emi/battle/batl_re2/01.bin; pass --size |
| `src/emi/battle/battle/03/func_801D527C.c` | `emi/battle/battle/03` | `0x801D527C` | unbound symbols: BATTLE_SLOT_BYTE_136 |
| `src/emi/battle/battle/03/func_801D5DCC.c` | `emi/battle/battle/03` | `0x801D5DCC` | unbound symbols: BATTLE_GLOBAL_BYTE_44F58, BATTLE_GLOBAL_BYTE_63C9, BATTLE_GLOBAL_HALF_63C2 |
| `src/emi/battle/battle/03/func_801D62D8.c` | `emi/battle/battle/03` | `0x801D62D8` | unbound symbols: BATTLE_GLOBAL_HALF_63B8, BATTLE_GLOBAL_HALF_63C2 |
| `src/emi/battle/battle/03/func_801D750C.c` | `emi/battle/battle/03` | `0x801D750C` | unbound symbols: BATTLE_GLOBAL_BYTE_4952, BATTLE_GLOBAL_BYTE_62F0, BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801D7A40.c` | `emi/battle/battle/03` | `0x801D7A40` | unbound symbols: BATTLE_GLOBAL_BYTE_62F4, BATTLE_GLOBAL_BYTE_6301, BATTLE_GLOBAL_BYTE_6302 |
| `src/emi/battle/battle/03/func_801D7D10.c` | `emi/battle/battle/03` | `0x801D7D10` | unbound symbols: BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801D7EB0.c` | `emi/battle/battle/03` | `0x801D7EB0` | unbound symbols: BATTLE_GLOBAL_BYTE_4952, BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801D8270.c` | `emi/battle/battle/03` | `0x801D8270` | unbound symbols: BATTLE_GLOBAL_BYTE_4952, BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801D8450.c` | `emi/battle/battle/03` | `0x801D8450` | unbound symbols: BATTLE_GLOBAL_BYTE_4952, BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801D8690.c` | `emi/battle/battle/03` | `0x801D8690` | unbound symbols: BATTLE_GLOBAL_BYTE_4952, BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801D8AE4.c` | `emi/battle/battle/03` | `0x801D8AE4` | unbound symbols: BATTLE_GLOBAL_BYTE_4952, BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801D8DF8.c` | `emi/battle/battle/03` | `0x801D8DF8` | unbound symbols: BATTLE_GLOBAL_BYTE_4952, BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801D9304.c` | `emi/battle/battle/03` | `0x801D9304` | unbound symbols: BATTLE_GLOBAL_BYTE_62F0, BATTLE_UI_BYTE_8356, BATTLE_UI_BYTE_8357, BATTLE_UI_BYTE_835C, BATTLE_UI_BYTE_835D, BATTLE_UI_BYTE_835E, BATTLE_UI_HALF_8358, BATTLE_UI_HALF_835A |
| `src/emi/battle/battle/03/func_801D9684.c` | `emi/battle/battle/03` | `0x801D9684` | unbound symbols: BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801D9900.c` | `emi/battle/battle/03` | `0x801D9900` | unbound symbols: BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801D99AC.c` | `emi/battle/battle/03` | `0x801D99AC` | unbound symbols: BATTLE_GLOBAL_WORD_598C, BATTLE_SCRATCH_BYTE_000, BATTLE_SCRATCH_BYTE_001, BATTLE_SCRATCH_BYTE_002 |
| `src/emi/battle/battle/03/func_801D9AB4.c` | `emi/battle/battle/03` | `0x801D9AB4` | unbound symbols: BATTLE_GLOBAL_BYTE_4952, BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801D9C80.c` | `emi/battle/battle/03` | `0x801D9C80` | unbound symbols: BATTLE_GLOBAL_BYTE_4952, BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801D9DBC.c` | `emi/battle/battle/03` | `0x801D9DBC` | unbound symbols: BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801D9E9C.c` | `emi/battle/battle/03` | `0x801D9E9C` | unbound symbols: BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801DA078.c` | `emi/battle/battle/03` | `0x801DA078` | unbound symbols: BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801DA4B4.c` | `emi/battle/battle/03` | `0x801DA4B4` | unbound symbols: BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801DA5A8.c` | `emi/battle/battle/03` | `0x801DA5A8` | unbound symbols: BATTLE_GLOBAL_WORD_598C |
| `src/emi/battle/battle/03/func_801DA7D4.c` | `emi/battle/battle/03` | `0x801DA7D4` | unbound symbols: BATTLE_GLOBAL_BYTE_6303 |
| `src/emi/battle/battle/03/func_801DB844.c` | `emi/battle/battle/03` | `0x801DB844` | unbound symbols: BATTLE_GLOBAL_BYTE_6324 |
| `src/emi/battle/battle/03/func_801DB9E4.c` | `emi/battle/battle/03` | `0x801DB9E4` | unbound symbols: BATTLE_GLOBAL_BYTE_6324 |
| `src/emi/battle/battle/03/func_801DBB78.c` | `emi/battle/battle/03` | `0x801DBB78` | unbound symbols: BATTLE_UI_BYTE_83C3 |
| `src/emi/battle/battle/03/func_801DC044.c` | `emi/battle/battle/03` | `0x801DC044` | unbound symbols: BATTLE_GLOBAL_BYTE_44F58, BATTLE_GLOBAL_HALF_63DA, BATTLE_GLOBAL_HALF_EC30C, BATTLE_LOCAL_BYTE_05, BATTLE_SCRATCH_HALF_000 |
| `src/emi/battle/battle/03/func_801DC894.c` | `emi/battle/battle/03` | `0x801DC894` | unbound symbols: BATTLE_GLOBAL_BYTE_EC324 |
| `src/emi/battle/battle/03/func_801DCAD8.c` | `emi/battle/battle/03` | `0x801DCAD8` | unbound symbols: BATTLE_GLOBAL_HALF_EC2EE, BATTLE_GLOBAL_HALF_EC30C |
| `src/emi/battle/battle/03/func_801DD29C.c` | `emi/battle/battle/03` | `0x801DD29C` | unbound symbols: BATTLE_GLOBAL_BYTE_62EE |
| `src/emi/battle/battle/03/func_801DEF0C.c` | `emi/battle/battle/03` | `0x801DEF0C` | unbound symbols: BATTLE_LOCAL_BYTE_62EC |
| `src/emi/battle/battle/03/func_801DEFE4.c` | `emi/battle/battle/03` | `0x801DEFE4` | unbound symbols: BATTLE_GLOBAL_BYTE_62E0, BATTLE_GLOBAL_BYTE_6325, BATTLE_GLOBAL_BYTE_6328 |
| `src/emi/battle/battle/03/func_801DF914.c` | `emi/battle/battle/03` | `0x801DF914` | unbound symbols: BATTLE_GLOBAL_BYTE_63C9 |
| `src/emi/battle/battle/03/func_801E0B64.c` | `emi/battle/battle/03` | `0x801E0B64` | unbound symbols: BATTLE_GLOBAL_BYTE_62F0 |
| `src/emi/battle/battle/03/func_801E1DD4.c` | `emi/battle/battle/03` | `0x801E1DD4` | unbound symbols: BATTLE_LOCAL_BYTE_05 |
| `src/emi/battle/battle/03/func_801E2170.c` | `emi/battle/battle/03` | `0x801E2170` | unbound symbols: BATTLE_GLOBAL_BYTE_62EA |
| `src/emi/battle/battle/03/func_801E25E0.c` | `emi/battle/battle/03` | `0x801E25E0` | unbound symbols: BATTLE_GLOBAL_BYTE_62EA, BATTLE_GLOBAL_BYTE_6384 |
| `src/emi/battle/battle/03/func_801E2E30.c` | `emi/battle/battle/03` | `0x801E2E30` | unbound symbols: BATTLE_GLOBAL_BYTE_44F58, BATTLE_GLOBAL_BYTE_62F0 |
| `src/emi/battle/battle/03/func_801E30B8.c` | `emi/battle/battle/03` | `0x801E30B8` | unbound symbols: BATTLE_GLOBAL_BYTE_62F3 |
| `src/emi/battle/battle/03/func_801E30F8.c` | `emi/battle/battle/03` | `0x801E30F8` | unbound symbols: BATTLE_LOCAL_FLAG_63CE |
| `src/emi/battle/battle/03/func_801E531C.c` | `emi/battle/battle/03` | `0x801E531C` | unbound symbols: BATTLE_GLOBAL_BYTE_6327 |
| `src/emi/battle/battle/03/func_801E54EC.c` | `emi/battle/battle/03` | `0x801E54EC` | unbound symbols: BATTLE_GLOBAL_BYTE_62F3, BATTLE_GLOBAL_BYTE_6328, BATTLE_GLOBAL_BYTE_63CA, BATTLE_GLOBAL_WORD_632C, BATTLE_GLOBAL_WORD_6330 |
| `src/emi/battle/battle/03/func_801E5824.c` | `emi/battle/battle/03` | `0x801E5824` | unbound symbols: BATTLE_CURRENT_QUEUED_WORD_4B20 |
| `src/emi/battle/battle/03/func_801E6C84.c` | `emi/battle/battle/03` | `0x801E6C84` | unbound symbols: BATTLE_GLOBAL_BYTE_62E0, memcpy |
| `src/emi/battle/battle/03/func_801E9074.c` | `emi/battle/battle/03` | `0x801E9074` | unbound symbols: memcpy |
| `src/emi/battle/battle/03/func_801EA650.c` | `emi/battle/battle/03` | `0x801EA650` | unbound symbols: memcpy |
| `src/emi/battle/battle/03/func_801EAAB8.c` | `emi/battle/battle/03` | `0x801EAAB8` | unbound symbols: BATTLE_UI_RING_INDEX |
| `src/emi/battle/battle/15/func_80096AB0.c` | `emi/battle/battle/15` | `0x80096AB0` | unbound symbols: battle_stage_attack_name_message |
| `src/emi/battle/battle/15/func_80096B24.c` | `emi/battle/battle/15` | `0x80096B24` | unbound symbols: BATTLE_SELECTION_PENDING_KIND, BATTLE_SELECTION_RING_RESET, BATTLE_SELECTION_SUBSTATE, battle_resolve_selection_slot |
| `src/emi/battle/battle/15/func_80096E14.c` | `emi/battle/battle/15` | `0x80096E14` | unbound symbols: battle_queue_frontend_cue |
| `src/emi/battle/battle/15/func_80096E90.c` | `emi/battle/battle/15` | `0x80096E90` | unbound symbols: battle_queue_frontend_cue, battle_stage_message_resource |
| `src/emi/battle/battle/15/func_80096F78.c` | `emi/battle/battle/15` | `0x80096F78` | unbound symbols: BATTLE_SELECTION_SUBSTATE, battle_resolve_frontend_resource, battle_stage_selection_ring_record |
| `src/emi/battle/battle/15/func_80096FBC.c` | `emi/battle/battle/15` | `0x80096FBC` | unbound symbols: BATTLE_SELECTION_ROOT_STATE |
| `src/emi/battle/battle/15/func_8009704C.c` | `emi/battle/battle/15` | `0x8009704C` | unbound symbols: BATTLE_INPUT_CANCEL_MASK, BATTLE_INPUT_CONFIRM_MASK, BATTLE_INPUT_HELD_MASK, BATTLE_PANEL_ICON_RING_HEAD, BATTLE_PANEL_STATE_KIND, BATTLE_SELECTION_CURSOR_BASE_X, BATTLE_SELECTION_CURSOR_BASE_Y, BATTLE_SELECTION_CURSOR_DIRTY, BATTLE_SELECTION_CURSOR_INDEX, BATTLE_SELECTION_CURSOR_X, BATTLE_SELECTION_CURSOR_Y, BATTLE_SELECTION_GROUP_INDEX, BATTLE_SELECTION_LOCKED, BATTLE_SELECTION_MOVE_SFX, BATTLE_SELECTION_OWNER_STATE, BATTLE_SELECTION_RING_INDEX, BATTLE_SELECTION_ROOT_STATE, BATTLE_SELECTION_SCROLL_BASE, BATTLE_SELECTION_SCROLL_DELTA, BATTLE_SELECTION_SOURCE_SLOT, battle_decode_repeatable_input, battle_queue_frontend_cue, battle_resolve_frontend_resource, battle_resolve_selection_kind_table, battle_selection_kind_is_blocked |
| `src/emi/battle/battle/15/func_8009761C.c` | `emi/battle/battle/15` | `0x8009761C` | unbound symbols: BATTLE_SELECTION_ROOT_STATE, BATTLE_SELECTION_SUBSTATE, battle_resolve_selection_kind_table |
| `src/emi/battle/battle/15/func_80097778.c` | `emi/battle/battle/15` | `0x80097778` | unbound symbols: BATTLE_SELECTION_PENDING_KIND, BATTLE_SELECTION_RING_RESET, BATTLE_SELECTION_SUBSTATE, battle_resolve_selection_slot, battle_result_uses_empty_slot |
| `src/emi/battle/battle/15/func_800980E4.c` | `emi/battle/battle/15` | `0x800980E4` | unbound symbols: BATTLE_SELECTION_PENDING_KIND, BATTLE_SELECTION_RING_RESET, BATTLE_SELECTION_ROOT_STATE, battle_resolve_selection_slot, battle_stage_attack_name_message |
| `src/emi/battle/battle/15/func_800983C4.c` | `emi/battle/battle/15` | `0x800983C4` | unbound symbols: BATTLE_SELECTION_SUBSTATE, battle_resolve_frontend_resource, battle_stage_selection_ring_record |
| `src/emi/battle/battle/15/func_80098450.c` | `emi/battle/battle/15` | `0x80098450` | unbound symbols: BATTLE_INPUT_CANCEL_MASK, BATTLE_INPUT_CONFIRM_MASK, BATTLE_INPUT_HELD_MASK, BATTLE_PANEL_ICON_RING_HEAD, BATTLE_PANEL_PROMPT_STATE, BATTLE_SECONDARY_CURSOR_INDEX, BATTLE_SECONDARY_MOVE_SFX, BATTLE_SECONDARY_PAGE_BASE, BATTLE_SECONDARY_PANEL_KIND, BATTLE_SECONDARY_PROMPT_ACTIVE, BATTLE_SECONDARY_PROMPT_CURSOR_LIMIT, BATTLE_SECONDARY_PROMPT_KIND, BATTLE_SECONDARY_PROMPT_MODE, BATTLE_SECONDARY_PROMPT_ROWS, BATTLE_SECONDARY_PROMPT_X, BATTLE_SECONDARY_PROMPT_Y, BATTLE_SECONDARY_SAVED_CURSOR, BATTLE_SECONDARY_SAVED_GROUP, BATTLE_SECONDARY_SAVED_PAGE_BASE, BATTLE_SECONDARY_SOURCE_GROUP, BATTLE_SELECTION_CURSOR_BASE_X, BATTLE_SELECTION_CURSOR_BASE_Y, BATTLE_SELECTION_CURSOR_DIRTY, BATTLE_SELECTION_CURSOR_X, BATTLE_SELECTION_CURSOR_Y, BATTLE_SELECTION_LOCKED, BATTLE_SELECTION_ROOT_STATE, BATTLE_SELECTION_SCROLL_DELTA, battle_decode_repeatable_input, battle_queue_frontend_cue, battle_resolve_frontend_resource, battle_resolve_secondary_choice_resource, battle_try_commit_secondary_choice |
| `src/emi/battle/battle/15/func_800989B4.c` | `emi/battle/battle/15` | `0x800989B4` | unbound symbols: BATTLE_SELECTION_LOCKED, BATTLE_SELECTION_OWNER_STATE, BATTLE_SELECTION_PHASE, BATTLE_SELECTION_ROOT_STATE |
| `src/emi/battle/battle/15/func_8009B20C.c` | `emi/battle/battle/15` | `0x8009B20C` | unbound symbols: battle_reset_local_task_slot |
| `src/emi/battle/battle/15/func_8009BBE8.c` | `emi/battle/battle/15` | `0x8009BBE8` | unbound symbols: BATTLE_PANEL_RULE_PASS_KIND, battle_copy_local_panel_rule_entry, battle_local_panel_slot_has_entry, battle_set_local_panel_slot_active |
| `src/emi/battle/battle/15/func_8009C8AC.c` | `emi/battle/battle/15` | `0x8009C8AC` | unbound symbols: BATTLE_PANEL_RULE_PASS_KIND, BATTLE_PANEL_RULE_PASS_SELECTION, BATTLE_PANEL_RULE_PASS_SLOT |
| `src/emi/battle/battle/15/func_8009CFEC.c` | `emi/battle/battle/15` | `0x8009CFEC` | unbound symbols: BATTLE_LOCAL_PANEL_ENTRY_COUNT, memcpy |
| `src/emi/etc/bate/03/func_80033A00.c` | `emi/etc/bate/03` | `0x80033A00` | 0x80033a00 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/bate/03.bin loaded at 0x801d0dd4 |
| `src/emi/etc/commu00/00/func_801EEDF8.c` | `emi/etc/commu00/00` | `0x801EEDF8` | 0x801eedf8..0x801eeef0 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801EEEF0.c` | `emi/etc/commu00/00` | `0x801EEEF0` | 0x801eeef0 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F00D4.c` | `emi/etc/commu00/00` | `0x801F00D4` | 0x801f00d4..0x801f01f4 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F01F4.c` | `emi/etc/commu00/00` | `0x801F01F4` | 0x801f01f4..0x801f02e4 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F02E4.c` | `emi/etc/commu00/00` | `0x801F02E4` | 0x801f02e4..0x801f0320 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F0320.c` | `emi/etc/commu00/00` | `0x801F0320` | 0x801f0320..0x801f0534 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F0534.c` | `emi/etc/commu00/00` | `0x801F0534` | 0x801f0534..0x801f0718 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F0718.c` | `emi/etc/commu00/00` | `0x801F0718` | 0x801f0718..0x801f08d8 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F08D8.c` | `emi/etc/commu00/00` | `0x801F08D8` | 0x801f08d8..0x801f0bf4 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F0BF4.c` | `emi/etc/commu00/00` | `0x801F0BF4` | 0x801f0bf4..0x801f0c6c is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F0C6C.c` | `emi/etc/commu00/00` | `0x801F0C6C` | 0x801f0c6c..0x801f0d3c is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F0D3C.c` | `emi/etc/commu00/00` | `0x801F0D3C` | 0x801f0d3c..0x801f0e1c is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F0E1C.c` | `emi/etc/commu00/00` | `0x801F0E1C` | 0x801f0e1c..0x801f0ec8 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F0EC8.c` | `emi/etc/commu00/00` | `0x801F0EC8` | 0x801f0ec8..0x801f0f08 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F0F08.c` | `emi/etc/commu00/00` | `0x801F0F08` | 0x801f0f08..0x801f0fbc is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F0FBC.c` | `emi/etc/commu00/00` | `0x801F0FBC` | 0x801f0fbc..0x801f1064 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F1064.c` | `emi/etc/commu00/00` | `0x801F1064` | 0x801f1064..0x801f1110 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F1110.c` | `emi/etc/commu00/00` | `0x801F1110` | 0x801f1110..0x801f1204 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F1204.c` | `emi/etc/commu00/00` | `0x801F1204` | 0x801f1204..0x801f1254 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F1254.c` | `emi/etc/commu00/00` | `0x801F1254` | 0x801f1254..0x801f18f8 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F18F8.c` | `emi/etc/commu00/00` | `0x801F18F8` | 0x801f18f8..0x801f1bc8 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/etc/commu00/00/func_801F1BC8.c` | `emi/etc/commu00/00` | `0x801F1BC8` | 0x801f1bc8 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/etc/commu00/00.bin loaded at 0x801d0c00 |
| `src/emi/scenario/scena00/00/func_801F7134.c` | `emi/scenario/scena00/00` | `0x801F7134` | 0x801f7134..0x801f78ec is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/scenario/scena00/00.bin loaded at 0x801d0dd4 |
| `src/emi/scenario/scena00/00/func_801F78EC.c` | `emi/scenario/scena00/00` | `0x801F78EC` | 0x801f78ec is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/scenario/scena00/00.bin loaded at 0x801d0dd4 |
| `src/emi/world00/area016/13/func_801F39D8.c` | `emi/world00/area016/13` | `0x801F39D8` | unbound symbols: GetGraphType |
| `src/emi/world00/area016/13/func_801F3B00.c` | `emi/world00/area016/13` | `0x801F3B00` | unbound symbols: GetGraphType, WORLD00_AREA016_GLOBAL_BYTE_832E, WORLD00_AREA016_GLOBAL_HALF_5AB4, WORLD00_AREA016_GLOBAL_HALF_5AC0, WORLD00_AREA016_GLOBAL_HALF_6258, WORLD00_AREA016_GLOBAL_HALF_625A, WORLD00_AREA016_GLOBAL_HALF_930A, WORLD00_AREA016_GLOBAL_HALF_930E, WORLD00_AREA016_STREAM_HINT |
| `src/emi/world00/area016/13/func_801F40C4.c` | `emi/world00/area016/13` | `0x801F40C4` | unbound symbols: WORLD00_AREA016_BOOT_HALF_0008, WORLD00_AREA016_GLOBAL_BYTE_832E |
| `src/emi/world00/area024/14/func_801F2FD4.c` | `emi/world00/area024/14` | `0x801F2FD4` | unbound symbols: VectorNormal, rand |
| `src/emi/world00/area024/14/func_801F3314.c` | `emi/world00/area024/14` | `0x801F3314` | unbound symbols: VectorNormalS |
| `src/emi/world00/area024/14/func_801F362C.c` | `emi/world00/area024/14` | `0x801F362C` | unbound symbols: WORLD00_AREA024_GLOBAL_HALF_3E6C |
| `src/emi/world00/area024/14/func_801F3708.c` | `emi/world00/area024/14` | `0x801F3708` | unbound symbols: RotMatrix, RotTrans, SetRotMatrix, SetTransMatrix, rand |
| `src/emi/world00/area024/14/func_801F3944.c` | `emi/world00/area024/14` | `0x801F3944` | unbound symbols: rcos, rsin |
| `src/emi/world00/area024/14/func_801F3BE4.c` | `emi/world00/area024/14` | `0x801F3BE4` | unbound symbols: ApplyMatrixSV, PopMatrix, PushMatrix, RotMatrixX, RotMatrixY, RotMatrixZ, rand, rcos, rsin |
| `src/emi/world00/area024/14/func_801F3E48.c` | `emi/world00/area024/14` | `0x801F3E48` | unbound symbols: PopMatrix, PushMatrix, RotTransPers, SetRotMatrix, SetSemiTrans, SetTransMatrix |
| `src/emi/world00/area026/13/func_801F2D5C.c` | `emi/world00/area026/13` | `0x801F2D5C` | 0x801f2d5c..0x801f2e04 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/world00/area026/13.bin loaded at 0x801f2e04 |
| `src/emi/world00/area026/13/func_801F2E04.c` | `emi/world00/area026/13` | `0x801F2E04` | unbound symbols: GetTPage, SetDrawMode, SetPolyF3, SetSemiTrans, rcos, rsin |
| `src/emi/world00/area027/13/func_801F2E3C.c` | `emi/world00/area027/13` | `0x801F2E3C` | unbound symbols: rcos, rsin |
| `src/emi/world00/area027/13/func_801F304C.c` | `emi/world00/area027/13` | `0x801F304C` | unbound symbols: rcos, rsin |
| `src/emi/world00/area027/13/func_801F33A8.c` | `emi/world00/area027/13` | `0x801F33A8` | unbound symbols: GetGraphType |
| `src/emi/world00/area028/13/func_801F2D3C.c` | `emi/world00/area028/13` | `0x801F2D3C` | 0x801f2d3c..0x801f2f5c is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/world00/area028/13.bin loaded at 0x801f2e04 |
| `src/emi/world00/area028/13/func_801F2FB0.c` | `emi/world00/area028/13` | `0x801F2FB0` | unbound symbols: rand |
| `src/emi/world00/area028/13/func_801F318C.c` | `emi/world00/area028/13` | `0x801F318C` | unbound symbols: WORLD00_AREA028_CENTER_X, WORLD00_AREA028_CENTER_Y, rcos, rsin |
| `src/emi/world00/area030/04/func_801D11C0.c` | `emi/world00/area030/04` | `0x801D11C0` | unbound symbols: WORLD00_AREA030_GLOBAL_BYTE_5E92, WORLD00_AREA030_GLOBAL_BYTE_5EBA, WORLD00_AREA030_GLOBAL_BYTE_5ED9, WORLD00_AREA030_GLOBAL_BYTE_5EDA, WORLD00_AREA030_GLOBAL_BYTE_5EDB, WORLD00_AREA030_GLOBAL_HALF_5EE8, WORLD00_AREA030_GLOBAL_HALF_5EEA, WORLD00_AREA030_GLOBAL_HALF_930E, WORLD00_AREA030_GLOBAL_WORD_5EE0, WORLD00_AREA030_GLOBAL_WORD_5EE4 |
| `src/emi/world00/area030/04/func_801D159C.c` | `emi/world00/area030/04` | `0x801D159C` | unbound symbols: WORLD00_AREA030_GLOBAL_BYTE_3FC9, WORLD00_AREA030_GLOBAL_BYTE_4002, WORLD00_AREA030_GLOBAL_BYTE_5E92, WORLD00_AREA030_GLOBAL_WORD_3E6C |
| `src/emi/world00/area030/04/func_801D1744.c` | `emi/world00/area030/04` | `0x801D1744` | unbound symbols: SetPolyG4 |
| `src/emi/world00/area030/04/func_801D1818.c` | `emi/world00/area030/04` | `0x801D1818` | unbound symbols: SetSprt16, WORLD00_AREA030_GLOBAL_WORD_3E6C |
| `src/emi/world00/area030/04/func_801D18CC.c` | `emi/world00/area030/04` | `0x801D18CC` | unbound symbols: SetSprt8 |
| `src/emi/world00/area030/04/func_801D195C.c` | `emi/world00/area030/04` | `0x801D195C` | unbound symbols: SetPolyF4, WORLD00_AREA030_GLOBAL_BYTE_3FC9, WORLD00_AREA030_GLOBAL_HALF_3FFC, WORLD00_AREA030_GLOBAL_HALF_4000, WORLD00_AREA030_GLOBAL_HALF_4006 |
| `src/emi/world00/area030/04/func_801D1B88.c` | `emi/world00/area030/04` | `0x801D1B88` | unbound symbols: SetPolyG4 |
| `src/emi/world00/area030/04/func_801D2C34.c` | `emi/world00/area030/04` | `0x801D2C34` | unbound symbols: GetGraphType, SetPolyFT4 |
| `src/emi/world00/area030/04/func_801D3244.c` | `emi/world00/area030/04` | `0x801D3244` | unbound symbols: SetTile, WORLD00_AREA030_GLOBAL_WORD_3E6C |
| `src/emi/world00/area032/13/func_801F2F04.c` | `emi/world00/area032/13` | `0x801F2F04` | 0x801f2f04 is outside /home/rcorreia/projects/rebof3-simple/out/binaries/emi/world00/area032/13.bin loaded at 0x801d11c0 |
