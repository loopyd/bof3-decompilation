#ifndef EMI_BATTLE_15_INTERNAL_H
#define EMI_BATTLE_15_INTERNAL_H

#include "bof3/bof3.h"
#include "panel/task.h"
#include "gpu/prim.h"
#include "battle/ram.h"

typedef void (*BattleSelectionHandler)(void);
typedef struct BattleSelectionDispatchTable {
  BattleSelectionHandler handlers[3];
} BattleSelectionDispatchTable;

typedef struct BattleSelectionAction {
  BattleSelectionHandler handler;
  u32 unk_04;
} BattleSelectionAction;

typedef struct BattlePanelTaskDispatchTable {
  BattleSelectionHandler handlers[5];
} BattlePanelTaskDispatchTable;

typedef struct BattleLocalPanelEntry {
  u8  owner_index;
  u8  unk_01;
  u16 panel_id;
} BattleLocalPanelEntry;

/* 0x118-byte local panel record: the kind byte read by func_8009CFEC (and by
 * the BATTLE_LOCAL_PANEL_OWNER_KIND view) comes first; the rest is unnamed. */
typedef struct BattleLocalPanelRecord {
  u8 kind;
  u8 unk_01[0x117];
} BattleLocalPanelRecord;

typedef struct BattlePanelTask {
  u8  unk_00[3];
  u8  state;
  u16 x;
  s16 field_06;
} BattlePanelTask;

typedef struct BattleSelectionKind {
  u16 mask;
  u8  unk_02[0x12];
} BattleSelectionKind;

typedef struct BattleSelectionFlags {
  u8 flags;
  u8 unk_01[0x13];
} BattleSelectionFlags;

typedef struct BattleWork {
  u8  unk_00[0x08];
  u8  unk_08;
  u8  unk_09[0x03];
  s32 unk_0C;
  s32 unk_10;
  u8  unk_14[0x1C];
  u16 unk_30;
  u16 unk_32;
  u32 range_axis_34;
  u32 range_axis_38;
  u8  unk_3C[0x02];
  u16 unk_3E;
} BattleWork;

typedef struct BattleRange {
  u8  unk_00[0x34];
  u32 range_axis_34;
  u32 range_axis_38;
  u8  unk_3C[0x02];
  u16 unk_3E;
} BattleRange;

typedef struct BattleLocalWork {
  u8 unk_00;
  u8 unk_01;
  u8 unk_02;
  u8 unk_03;
  u8 unk_04;
  u8 unk_05[0x81];
  u8 unk_86;
  u8 unk_87;
  u8 unk_88[0x9C];
  /* Flag word read by func_800A99AC at 0x80145FB4 (record + 0x124). */
  u32 unk_124;
  u8 unk_128[0x18];
} BattleLocalWork;

typedef struct BattleStatusSlots {
  u8 slots[10];
  u8 unk_0A[0x136];
} BattleStatusSlots;

typedef struct BattleLocalOffsetPair {
  s16 values[4][2];
} BattleLocalOffsetPair;

typedef struct Unk801EBF08 {
  u8  unk_00;
  u8  unk_01;
  u8  unk_02[0x126];
  volatile u32 unk_128;
} Unk801EBF08;

typedef struct BattleRecord {
  u8 unk_00;
  u8 unk_01;
  u8 unk_02;
  u8 unk_03;
  u8 unk_04;
  u8 unk_05[0xFB];
  /* Flag word read by func_800A99AC at 0x801EB730 (record + 0x100). */
  u32 unk_100;
  u8 unk_104[0x14];
} BattleRecord;

extern PanelTask*   D_80148648; /* @source 0x80148648 @kind unknown */

/* Scratchpad work-area pointer (volatile cell at 0x1F800044).
 * Reloaded per access to match original codegen. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */
extern BattleSelectionDispatchTable D_80096994; /* @source 0x80096994 @kind unknown */
extern BattleSelectionDispatchTable D_800969A0; /* @source 0x800969A0 @kind unknown */
extern BattleSelectionDispatchTable D_800969AC; /* @source 0x800969AC @kind unknown */
extern BattleSelectionDispatchTable D_800969B8; /* @source 0x800969B8 @kind unknown */
extern volatile u32 D_801459F0; /* @source 0x801459F0 @kind unknown */
extern s8 D_800B4E8C[]; /* @source 0x800B4E8C @kind unknown */
extern u8 D_800B4E94[]; /* @source 0x800B4E94 @kind table */
extern u8 D_800B4E99[]; /* @source 0x800B4E99 @kind table */
extern u8 D_800B4E9E[]; /* @source 0x800B4E9E @kind table */
extern u8 D_800B4C7C[]; /* @source 0x800B4C7C @kind unknown */
extern u8 D_800B4D58[][3]; /* @source 0x800B4D58 @kind table */
extern BattleSelectionHandler D_800B6BF4[]; /* @source 0x800B6BF4 @kind unknown */
extern BattleRecord D_801EB630[]; /* @source 0x801EB630 @kind unknown */
extern u8 D_801463B8; /* @source 0x801463B8 @kind unknown */
extern u8 D_801463C4[]; /* @source 0x801463C4 @kind unknown */
extern u8 D_801463C7; /* @source 0x801463C7 @kind unknown */
typedef struct BattleIdentityRecord {
  u8 id;
  u8 unk_01[0x87];
} BattleIdentityRecord;

extern BattleIdentityRecord D_800E4050[]; /* @source 0x800E4050 @kind unknown */

u32 func_800AF66C(BattleRange *range, u32 value);
u32 func_800A99AC(void);
u8 *findOwnerRecord(u8 id);

/* Absolute-address globals. Bound via WEAK_SYMBOL_AT in symbols.c; values
 * equal the symbol-name addresses. */
extern BattlePanelTaskDispatchTable D_800969E4; /* @source 0x800969E4 @kind unknown */
extern BattlePanelTaskDispatchTable D_80096A14; /* @source 0x80096A14 @kind unknown */
extern BattleSelectionDispatchTable D_800969F8; /* @source 0x800969F8 @kind unknown */
extern BattleSelectionDispatchTable D_80096A08; /* @source 0x80096A08 @kind unknown */
extern BattleSelectionDispatchTable D_80096A34; /* @source 0x80096A34 @kind unknown */
extern BattleSelectionDispatchTable D_80096A40; /* @source 0x80096A40 @kind unknown */
extern s32 D_80144F60[]; /* @source 0x80144F60 @kind unknown */
extern volatile u32  D_80144F80[]; /* @source 0x80144F80 @kind unknown */
extern u16 D_80145FAA[]; /* @source 0x80145FAA @kind unknown */
/* Shared runtime flag byte raised by func_800A4BB4 once the EXE-side EMI
 * loader reports ready; its consumer is outside this target. */
extern u8 D_80145988; /* @source 0x80145988 @kind unknown */
extern volatile u32  D_80146250; /* @source 0x80146250 @kind unknown */
extern volatile u8   D_801462E0; /* @source 0x801462E0 @kind unknown */
extern volatile u8   D_801462E1; /* @source 0x801462E1 @kind unknown */
extern volatile u8   D_801462E2; /* @source 0x801462E2 @kind unknown */
extern volatile u8   D_801462E3; /* @source 0x801462E3 @kind unknown */
extern BattleSelectionHandler D_800B43C0[]; /* @source 0x800B43C0 @kind unknown */
extern BattleSelectionHandler D_800B4450[]; /* @source 0x800B4450 @kind unknown */
extern BattleSelectionHandler D_800B5108[]; /* @source 0x800B5108 @kind unknown */
extern BattleSelectionHandler D_800B43D4[]; /* @source 0x800B43D4 @kind unknown */
extern BattleSelectionHandler battleSelectionHandlerTable4CAC[]; /* @source 0x800B4CAC @kind table */
extern BattleSelectionHandler battleSelectionHandlerTable4CC8[]; /* @source 0x800B4CC8 @kind table */
extern BattleSelectionHandler battleSelectionHandlerTable4CD0[]; /* @source 0x800B4CD0 @kind table */
extern BattleSelectionHandler battleSelectionHandlerTable4CE4[]; /* @source 0x800B4CE4 @kind table */
extern BattleSelectionHandler battleSelectionHandlerTable4D00[]; /* @source 0x800B4D00 @kind table */
extern BattleSelectionHandler battleSelectionHandlerTable4D14[]; /* @source 0x800B4D14 @kind table */
extern BattleSelectionHandler battleSelectionHandlerTable4D30[]; /* @source 0x800B4D30 @kind table */
extern BattleSelectionHandler D_800B43EC[]; /* @source 0x800B43EC @kind unknown */
extern BattleSelectionHandler D_800B43F4[]; /* @source 0x800B43F4 @kind unknown */
extern BattleSelectionHandler D_800B4408[]; /* @source 0x800B4408 @kind unknown */
extern BattleSelectionHandler D_800B4418[]; /* @source 0x800B4418 @kind unknown */
extern BattleSelectionHandler D_800B44A0[]; /* @source 0x800B44A0 @kind unknown */
extern BattleSelectionHandler D_800B4428[]; /* @source 0x800B4428 @kind unknown */
extern BattleSelectionHandler D_800B4458[]; /* @source 0x800B4458 @kind unknown */
extern BattleSelectionHandler D_800B446C[]; /* @source 0x800B446C @kind unknown */
extern BattleSelectionHandler D_800B447C[]; /* @source 0x800B447C @kind unknown */
extern BattleSelectionHandler D_800B448C[]; /* @source 0x800B448C @kind unknown */
extern BattleSelectionHandler battleSelectionHandlerTable44C8[]; /* @source 0x800B44C8 @kind table */
extern BattleSelectionHandler D_800B44D4[]; /* @source 0x800B44D4 @kind unknown */
extern BattleSelectionHandler battleSelectionHandlerTable44E4[]; /* @source 0x800B44E4 @kind table */
extern BattleSelectionHandler battlePanelOuterStateHandlerTable[]; /* @source 0x800B6E08 @kind table */
extern BattleSelectionHandler battlePanelStateHandlerTable6e20[]; /* @source 0x800B6E20 @kind table */
extern BattleSelectionHandler battlePanelStateHandlerTable6e34[]; /* @source 0x800B6E34 @kind table */
extern BattleSelectionHandler battlePanelStateHandlerTable6e50[]; /* @source 0x800B6E50 @kind table */
extern BattleLocalOffsetPair D_800B6C90[]; /* @source 0x800B6C90 @kind unknown */
extern u8 D_800B6D00[]; /* @source 0x800B6D00 @kind unknown */
extern u8 D_801462E4; /* @source 0x801462E4 @kind unknown */
extern u8 D_801462EA; /* @source 0x801462EA @kind unknown */
extern volatile u8   D_801462EF; /* @source 0x801462EF @kind unknown */
extern volatile u8   D_80146303; /* @source 0x80146303 @kind unknown */
extern volatile u16  D_80145AC8; /* @source 0x80145AC8 @kind unknown */
extern u8*           D_801EB4D8; /* @source 0x801EB4D8 @kind unknown */
/* Pointer cell holding the current local battle-work record; its byte 0x01 is
 * the byte compared with 5 by pollReadyThenInvokeHelperWhenWorkByte1Is5. */
extern BattleLocalWork* D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u8            D_801EBEFA[]; /* @source 0x801EBEFA @kind unknown */
extern u8*           D_801EBF08; /* @source 0x801EBF08 @kind unknown */
extern u8            D_80148330[]; /* @source 0x80148330 @kind unknown */
extern u8            D_801462E5; /* @source 0x801462E5 @kind unknown */
extern u8            D_801462E6; /* @source 0x801462E6 @kind unknown */
extern BattleSelectionAction battleSelectionActionTable[]; /* @source 0x800B65FC @kind table */
extern volatile u16  D_801462E8; /* @source 0x801462E8 @kind unknown */
/* Shared pending-battler bitset read by raiseFlag4WhenPendingBitsClear; the
 * sibling lifts at 0x801DE1B0 and 0x801DE1?? set and clear one bit per
 * battler index. */
extern u16           D_801463C2; /* @source 0x801463C2 @kind unknown */
extern volatile BattleLocalWork D_80145E90[]; /* @source 0x80145E90 @kind unknown */
extern BattleStatusSlots D_80145F7E[]; /* @source 0x80145F7E @kind unknown */
typedef struct BattleActiveRecord {
  u8 status;
  u8 unk_01[3];
  u32 flags;
  u8 unk_08[0x138];
} BattleActiveRecord;

typedef struct BattleReserveRecord {
  u8 status;
  u8 unk_01[3];
  u32 flags;
  u8 unk_08[0x110];
} BattleReserveRecord;

extern BattleActiveRecord D_80145FB0[]; /* @source 0x80145FB0 @kind unknown */
extern u8  D_801EB2E8[]; /* @source 0x801EB2E8 @kind unknown */
extern BattleReserveRecord D_801EB72C[]; /* @source 0x801EB72C @kind unknown */
extern u8  D_801EC337[]; /* @source 0x801EC337 @kind unknown */
extern u8  D_801EC33B[]; /* @source 0x801EC33B @kind unknown */
extern u8  D_801EC357[]; /* @source 0x801EC357 @kind unknown */
extern u8  D_801EC390[]; /* @source 0x801EC390 @kind unknown */
extern u8  D_801EC3A4[]; /* @source 0x801EC3A4 @kind unknown */
extern volatile u8             D_80146329; /* @source 0x80146329 @kind unknown */
extern volatile u8             D_80146374; /* @source 0x80146374 @kind unknown */
extern u8                      D_80146375; /* @source 0x80146375 @kind unknown */
extern volatile s8   D_80145558; /* @source 0x80145558 @kind unknown */
extern volatile s16  D_801EC2EE; /* @source 0x801EC2EE @kind unknown */
extern volatile u8   D_80146394; /* @source 0x80146394 @kind unknown */
extern s16* volatile D_801463A0; /* @source 0x801463A0 @kind unknown */
extern volatile u16  D_801463C0; /* @source 0x801463C0 @kind unknown */
extern u16 D_801EC2F2; /* @source 0x801EC2F2 @kind unknown */
extern u16 D_801EC312; /* @source 0x801EC312 @kind unknown */
extern u8 D_801CA71B[]; /* @source 0x801CA71B @kind unknown */
extern u8 D_80145F34[]; /* @source 0x80145F34 @kind unknown */
extern u8 D_80145F35[]; /* @source 0x80145F35 @kind unknown */
extern u8 D_80145F36[]; /* @source 0x80145F36 @kind unknown */
extern u8 D_80145F37[]; /* @source 0x80145F37 @kind unknown */
extern u8 D_80145F18[]; /* @source 0x80145F18 @kind unknown */
extern u8 D_80145F12[]; /* @source 0x80145F12 @kind unknown */
extern u8 D_80145F04[]; /* @source 0x80145F04 @kind unknown */
extern u8 D_801EB6E4[]; /* @source 0x801EB6E4 @kind unknown */
extern u8 D_801EB6E5[]; /* @source 0x801EB6E5 @kind unknown */
extern u8 D_801EB6E6[]; /* @source 0x801EB6E6 @kind unknown */
extern u8 D_801EB6E7[]; /* @source 0x801EB6E7 @kind unknown */
extern u8 D_801EB6A4[]; /* @source 0x801EB6A4 @kind unknown */
extern u8 D_801EB712[]; /* @source 0x801EB712 @kind unknown */
extern u8 D_801EB713[]; /* @source 0x801EB713 @kind unknown */
extern u8 D_801EB6C4[]; /* @source 0x801EB6C4 @kind unknown */
/* Local panel entry list read by func_8009CFEC: count byte plus 4-byte entries. */
extern u8 D_801EB5A8; /* @source 0x801EB5A8 @kind unknown */
extern BattleLocalPanelEntry D_801ED9B0[]; /* @source 0x801ED9B0 @kind unknown */
/* Kind byte of the 0x118-byte local panel record selected by an entry's owner
 * index (same record base the BATTLE_LOCAL_PANEL_* accessors describe). */
extern BattleLocalPanelRecord D_801EB6AC[]; /* @source 0x801EB6AC @kind unknown */
extern s16 D_800B494C[]; /* @source 0x800B494C @kind unknown */
extern s16 D_800B495C[]; /* @source 0x800B495C @kind unknown */
extern s16 D_800B496C[]; /* @source 0x800B496C @kind unknown */
extern s16 D_800B497C[]; /* @source 0x800B497C @kind unknown */
extern u8            D_801463C9; /* @source 0x801463C9 @kind unknown */
extern volatile u8   D_80146384; /* @source 0x80146384 @kind unknown */
extern volatile u8   D_801463CA; /* @source 0x801463CA @kind unknown */
extern volatile u8   D_801463CB; /* @source 0x801463CB @kind unknown */
extern volatile BattleSelectionFlags D_801CA718[]; /* @source 0x801CA718 @kind unknown */
extern volatile BattleSelectionKind D_801CA71C[]; /* @source 0x801CA71C @kind unknown */
extern u8 D_801C90EB[]; /* @source 0x801C90EB @kind unknown */
extern volatile u8   D_8014837B; /* @source 0x8014837B @kind unknown */
extern volatile u8   D_8014839F; /* @source 0x8014839F @kind unknown */
extern volatile u8   D_801483C3; /* @source 0x801483C3 @kind unknown */
extern volatile u8   D_80148597; /* @source 0x80148597 @kind unknown */
extern volatile u8   D_801485BB; /* @source 0x801485BB @kind unknown */
typedef struct BattleSelectionState {
  u8  first;
  u8  unk_01;
  u8  unk_02;
  u8  unk_03;
  s16 unk_04;
  s16 unk_06;
  u8  unk_08;
  u8  unk_09;
  u8  unk_0A;
  u8  unk_0B;
  u8  unk_0C;
  u8  unk_0D;
  u8  unk_0E[6];
  s16 unk_14;
  u8  unk_16[0x56];
  u8  field_6C;
} BattleSelectionState;

extern BattleSelectionState D_80148570; /* @source 0x80148570 @kind unknown */
extern u8 D_80148571; /* @source 0x80148571 @kind unknown */
extern u8 D_80148572; /* @source 0x80148572 @kind unknown */
extern u8 D_80148573; /* @source 0x80148573 @kind unknown */
extern s16 D_80148574; /* @source 0x80148574 @kind unknown */
extern s16 D_80148576; /* @source 0x80148576 @kind unknown */
extern u8 D_80148595; /* @source 0x80148595 @kind unknown */
extern u8 D_80148596; /* @source 0x80148596 @kind unknown */
extern u16 D_80148598; /* @source 0x80148598 @kind unknown */
extern u16 D_8014859A; /* @source 0x8014859A @kind unknown */
extern u8 D_8014859C; /* @source 0x8014859C @kind unknown */
extern u8 D_8014859D; /* @source 0x8014859D @kind unknown */
extern u8 D_8014859E; /* @source 0x8014859E @kind unknown */
extern u8 D_8014859F; /* @source 0x8014859F @kind unknown */
extern u8 D_801485A0; /* @source 0x801485A0 @kind unknown */
extern u8 D_801485A1; /* @source 0x801485A1 @kind unknown */
extern u8 D_80148578; /* @source 0x80148578 @kind unknown */
extern u8 D_80148579; /* @source 0x80148579 @kind unknown */
extern u8 D_8014857A; /* @source 0x8014857A @kind unknown */
extern u8 D_8014857B; /* @source 0x8014857B @kind unknown */
extern u8 D_8014857C; /* @source 0x8014857C @kind unknown */
extern u8 D_8014857D; /* @source 0x8014857D @kind unknown */
extern s16 D_80148580; /* @source 0x80148580 @kind unknown */
extern s16 D_80148584; /* @source 0x80148584 @kind unknown */
extern u8 D_801485DD; /* @source 0x801485DD @kind unknown */
extern volatile u8 D_801485DE; /* @source 0x801485DE @kind unknown */
extern volatile u8   D_801485DF; /* @source 0x801485DF @kind unknown */
extern volatile u16  D_801485E0; /* @source 0x801485E0 @kind unknown */
extern volatile u16  D_8014932E; /* @source 0x8014932E @kind unknown */
extern volatile u16  D_801485E2; /* @source 0x801485E2 @kind unknown */
extern s16 D_801485C8; /* @source 0x801485C8 @kind unknown */
extern s16 D_801485CA; /* @source 0x801485CA @kind unknown */
extern s16 D_801485EC; /* @source 0x801485EC @kind unknown */
extern s16 D_801485EE; /* @source 0x801485EE @kind unknown */
extern u8 D_80145500[][4]; /* @source 0x80145500 @kind unknown */
extern u8 D_80145518[][4]; /* @source 0x80145518 @kind unknown */
extern u8 D_80148624; /* @source 0x80148624 @kind unknown */
extern volatile u8   D_80148626; /* @source 0x80148626 @kind unknown */
extern volatile u8   D_80148627; /* @source 0x80148627 @kind unknown */
extern volatile s16  D_80148628; /* @source 0x80148628 @kind unknown */
extern volatile u16  D_8014862A; /* @source 0x8014862A @kind unknown */
extern volatile u8   D_8014862E; /* @source 0x8014862E @kind unknown */

/* Shared primitive cursor (PsyQ SDK, owned by the main exe). */
extern u8* g_PrimCursor; /* @source 0x8014598C @kind unknown */

/* PsyQ SDK primitive arena bookkeeping shared with appendRenderPrim. */
extern u8 D_80143D44; /* @source 0x80143D44 @kind unknown */
extern u8 D_80142CC4[]; /* @source 0x80142CC4 @kind unknown */

/* Reference cell read as two s16 values by func_800AF0F8. */
extern s16 D_80149320; /* @source 0x80149320 @kind unknown */
extern s16 D_80149322; /* @source 0x80149322 @kind unknown */

/* PsyQ SDK primitive setup helpers called by this target.
 * SetSprt8 / SetSemiTrans are declared by <libgpu.h> (via bof3/psyq.h);
 * func_8014E5A0 is a game primitive-append helper (lifted in exe/slus_004.22). */
void func_8014E5A0(u32 ot_index, u32 primitive_size);
/* INFERRED: 0x8014E3A0 is reached only from func_800A4BB4, which passes the
 * literal 0x1B in $a0; the callee itself lives outside this target, so the
 * parameter is modelled as an engine action id. Promotion needs the EXE-side
 * body of 0x8014E3A0. */
void func_8014E3A0(u32 action);
void battle_stage_attack_name_message(s32 slot_index, s32 queue_kind);
u8   battle_resolve_selection_slot(u32 family_id);
void battle_queue_frontend_cue(u32 cue_id);
u32  battle_resolve_frontend_resource(u16 resource_id);
void battle_stage_selection_ring_record(u32 slot_index, u32 record_kind,
                                        u32 resource_handle);
u32  battle_decode_repeatable_input(u16 input_mask);
u8*  battle_resolve_selection_kind_table(u32 source_slot, u32 group_index,
                                         u32 table_kind);
u8   battle_selection_kind_is_blocked(void);
void battle_reset_local_task_slot(void);
void battle_stage_message_resource(void* message_slot);
u8   battle_result_uses_empty_slot(void);
u8   battle_local_panel_slot_has_entry(volatile u8* battler, u32 slot_index);
void battle_copy_local_panel_rule_entry(volatile u8* battler,
                                        volatile u8* panel_rule);
void battle_set_local_panel_slot_active(volatile u8* battler, u32 slot_index,
                                        u32 active_state);
u16  battle_resolve_secondary_choice_resource(u32 group_index, u32 choice_id);
u8   battle_try_commit_secondary_choice(u32 panel_kind, u32 zero_arg,
                                        u32 group_index, u32 choice_id);
void                           resetStateWhenUnlocked(void);
void                           dispatchLocalHandlerPair(void);
void                           setWorkByte9Advance(void);
void                           func_800AE09C(void);
void                           dispatchWorkByte1Pair(void);
void                           initRecordStateAdvanceWork(void);
void                           raiseFlag4WhenPendingBitsClear(void);
void                           pollReadyThenInvokeHelperWhenWorkByte1Is5(void);
void                           resetStateWhenUnlockedB(void);
void __attribute__((noinline)) runPanelTasks16To19(void);
u8                             func_8009C8AC(u16 required_mask);
u32                            func_800A7B40(u8 group, u8 slot);
void                           func_8009CFEC(void);
void                           func_8009DCC8(void);
void                           func_8009DE18(void);
void                           func_8009DE8C(void);
void                           func_8009E500(void);
void                           func_8009F320(void);
void                           func_800A0378(void);
s16  func_800A2880(u8 battler_index, u16 base_value, u8 element_flag, u32 mode);
s32  passesBytePairGate(u8 battler_index, u8 value);
s16  func_800A2AE0(u8 battler_index, u16 element_mask);
s16  func_800A2D70(s32 battler_index, s32 selection_mask);
s32  func_800A30E8(u32 unused, u8 selector);
void func_800A31E0(u8 selection_kind, u16 input_mask);
s32  func_800A3A10(s32 battler_index, s32 selection_kind);
/* Selection roll: forwards (battler_index & 0xFF, selection_mask & 0x1FF) to
 * func_800A2D70 and returns a 0/1 success flag; its first parameter is not read
 * by the decoded body at 0x800A3B6C. */
s32  func_800A3B6C(s32 arg0, s32 battler_index, s32 selection_mask);
u16  func_800A36F0(u8 battler_index, u16 flags);
void func_800A3F28(void);
void func_800ABF5C(void);
void func_800ABBE0(void);
void func_800ABC30(void);
void func_800A9BD8(u8 battler_index);
void clampIndexedBattleByte14(s16 delta, s32 index);
u8   resetSelectionApplyInput(s32 input_mask);
u8   func_801DB524(u8 arg0);
s16  func_800A4294(s32 kind);
void func_800A4458(void);
void setupMode104ArmWorkBit2(void);
void func_800AAA74(void);
void func_800AAEBC(s16 target_index, u8 battler_index);
void func_800B0498(void);
void func_800B0B0C(s16 base_x, s16 base_y);
void func_800B0F9C(void);
void func_800B1728(void);
void func_800B2218(void);
void func_800B22AC(void);
void func_800B23B8(void);
void func_800B23F8(void);
void func_800B250C(void);
void func_800B25C0(void* panel);
void func_800B2C90(void* panel);
void func_800B3AE8(void* panel);
void func_800B4220(s16 arg0, s16 arg1, u8 arg2);
void setFlag2000StoreDoubledResult(void);
u8   querySelectionApplyInput(s32 input_mask);
s16  func_801DC044(u8 arg0, u8 arg1, u32 arg2);
void func_801647C4(u16 arg0, u16 arg1, s32 arg2);
void func_801DE94C(s32 arg0, s32 arg1);
void func_80158E20(void);
void func_8015DF18(u16 arg0);
u32  func_801502D0(u32 arg0);
void func_801DE8C0(u8 arg0, u8 arg1, u32 arg2);
u8   func_801DB5CC(s32 arg0);
void func_801E5988(void);
void func_801DEE4C(void);
u32  func_801E590C(u32 arg0, u32 arg1);
void func_801DEA64(s32 arg0);

#define BATTLE_ACTIVE_SELECTION_SLOT_PTR D_801EB4D8
#define BATTLE_ACTIVE_MESSAGE_SLOT_PTR   PSX_REF(volatile void*, 0x801ebf08u)
#define D_801EBF08_PTR                   ((Unk801EBF08*)D_801EBF08)
#define BATTLE_CURRENT_BATTLER_PTR       PSX_REF(volatile u8*, 0x801eb4e8u)
#define BATTLE_SELECTION_SLOT_SUBSTATE_TABLE                                   \
  PSX_PTR(const volatile BattleSelectionHandler, 0x800b43c0u)
#define BATTLE_SELECTION_CONFIRM_SUBSTATE_TABLE                                \
  PSX_PTR(const volatile BattleSelectionHandler, 0x800b43ecu)
#define BATTLE_SELECTION_RESULT_SUBSTATE_TABLE                                 \
  PSX_PTR(const volatile BattleSelectionHandler, 0x800b43f4u)
#define BATTLE_SELECTION_FINALIZE_SUBSTATE_TABLE                               \
  PSX_PTR(const volatile BattleSelectionHandler, 0x800b4408u)
#define BATTLE_SELECTION_SECONDARY_SUBSTATE_TABLE                              \
  PSX_PTR(const volatile BattleSelectionHandler, 0x800b4450u)
#define BATTLE_SELECTION_RING_FLAG(index)                                      \
  PSX_REF(volatile u8, 0x801eb5b1u + ((index) * 8u))
#define BATTLE_SELECTION_RING_HANDLE(index)                                    \
  PSX_REF(volatile u32, 0x801eb5b4u + ((index) * 8u))
#define BATTLE_SELECTION_SAVED_GROUP(index)                                    \
  PSX_REF(volatile u8, 0x801454f4u + ((index) * 3u))
#define BATTLE_SELECTION_SAVED_SCROLL(index)                                   \
  PSX_REF(volatile u8, 0x801454f5u + ((index) * 3u))
#define BATTLE_SELECTION_SAVED_CURSOR(index)                                   \
  PSX_REF(volatile u8, 0x801454f6u + ((index) * 3u))
#define BATTLE_SELECTION_KIND_FLAGS(kind)                                      \
  PSX_REF(volatile u8, 0x801ca718u + ((kind) * 0x14u))
#define BATTLE_SELECTION_KIND_MASK(kind)                                       \
  PSX_REF(volatile u16, 0x801ca71cu + ((kind) * 0x14u))
#define BATTLE_SELECTION_KIND_NAME_ID(kind)                                    \
  PSX_REF(volatile u16, 0x801ca71eu + ((kind) * 0x14u))
#define D_80148648 g_PanelTaskRoot
#define BATTLE_SECONDARY_GROUP_TABLE(index)                                    \
  PSX_REF(volatile u8*, 0x801c893cu + ((index) * 4u))
#define BATTLE_SELECTION_PANEL_FLAGS(index)                                    \
  PSX_REF(volatile u32, 0x80145fb4u + ((index) * 0x140u))
#define BATTLE_LOCAL_PANEL_RULE(class_id, slot_index)                          \
  PSX_PTR(volatile u8, (0x800e407cu + ((u32)(class_id) * 0x88u) +              \
                        ((u32)(slot_index) * 0x10u)))
#define BATTLE_PANEL_SLOT_KIND(slot_index)                                     \
  PSX_REF(volatile u8, 0x80145f12u + ((u32)(slot_index) * 0x140u))
#define BATTLE_PANEL_SLOT_MASK(kind)                                           \
  PSX_REF(volatile u8, 0x801c90ebu + ((u32)(kind) * 0x18u))
#define BATTLE_LOCAL_PANEL_OWNER_KIND(index)                                   \
  PSX_REF(volatile u8, 0x801eb6acu + ((u32)(index) * 0x118u))
/* Unknown bytes of the 0x118-byte local panel record selected by the index.
 * Declared as named symbols (not raw constants): the original encodes each store
 * as one symbolic addend plus the scaled index, which a volatile/raw-constant
 * access would materialize into a pooled `lui`+`ori` base instead. */
#define BATTLE_LOCAL_PANEL_UNK_6E(index)                                       \
  D_801EB712[((u32)(index) * 0x118u)]
#define BATTLE_LOCAL_PANEL_UNK_6F(index)                                       \
  D_801EB713[((u32)(index) * 0x118u)]
#define BATTLE_LOCAL_PANEL_ENTRY(index)                                        \
  PSX_REF(volatile BattleLocalPanelEntry, 0x801ed9b0u + ((u32)(index) * 4u))

/* Fixed-address bases, tables, and rodata pointers. The raw literals live only
 * here; function bodies reference these named accessors. */
#define BATTLE_GAME_RAM_BASE PSX_PTR(volatile u8, 0x80140000u)
#define BATTLE_LOCK_RAM_BASE PSX_PTR(volatile u8, 0x80150000u)
#define BATTLE_SELECTION_TABLE_BASE                                            \
  PSX_PTR(const volatile BattleSelectionHandler, 0x800b0000u)
#define BATTLE_MODIFIER_TABLE      PSX_PTR(volatile s16, 0x800b493cu)
#define BATTLE_TEMPLATE_BASE       PSX_PTR(volatile u8, 0x801ebef0u)
#define BATTLE_UNK_80148570_BASE   PSX_PTR(volatile u8, 0x80148570u)
#define BATTLE_UNK_801485DC        PSX_REF(volatile u8, 0x801485dcu)
#define BATTLE_UNK_801485DD        PSX_REF(volatile u8, 0x801485ddu)
#define BATTLE_UNK_801485DE        PSX_REF(volatile u8, 0x801485deu)
#define BATTLE_UNK_80148656        PSX_REF(volatile u8, 0x80148656u)
/* UNREVIEWED: the "palette" names below come from the partial lift
 * func_800B0B0C and are not proven. Original bytes at 0x800b6d20/0x800b6d28/
 * 0x800b6d30 are the ASCII string run "1"/"0"/"Data"/"Pick"/"Best"
 * continuing battleApParamStrings (0x800b6d1c, "AP"), inside the reviewed
 * pointer/string-table region (reviewed.rz: Cd 0x2D0C @ 0x800B43B8).
 * Rename only with evidence from a matched func_800B0B0C. */
#define BATTLE_PALETTE_TABLE       PSX_PTR(volatile u16, 0x800b6d30u)
#define BATTLE_PALETTE_ROW_28      PSX_PTR(volatile void, 0x800b6d28u)
#define BATTLE_PALETTE_ROW_20      PSX_PTR(volatile void, 0x800b6d20u)
/* @kind: string (map symbol: battleApParamStrings) — "AP" label + params string run. */
#define BATTLE_UNK_800B6D1C        PSX_PTR(volatile u32, 0x800b6d1cu)
/* @kind: string (map symbol: battlePanelIndexFormat) — "%2d" battler-index format. */
#define BATTLE_UNK_80096A04        PSX_PTR(volatile void, 0x80096a04u)
#define BATTLE_UNK_80145AD4        PSX_PTR(volatile u16, 0x80145ad4u)
#define BATTLE_SCRATCHPAD_PTR      SPAD_PTR_SLOT(u8, 0x44u)
#define BATTLE_PLAYER_BATTLER_BASE PSX_PTR(volatile u8, 0x80145e90u)
#define BATTLE_ENEMY_BATTLER_BASE  PSX_PTR(volatile u8, 0x801eb2e8u)
#define BATTLE_UNK_80145F44        PSX_PTR(volatile u16, 0x80145f44u)
#define BATTLE_UNK_80145F46        PSX_PTR(volatile u16, 0x80145f46u)
#define BATTLE_UNK_80145F59        PSX_PTR(volatile u8, 0x80145f59u)
#define BATTLE_UNK_80145F4A        PSX_PTR(volatile u16, 0x80145f4au)
#define BATTLE_UNK_801461CA        PSX_PTR(volatile u16, 0x801461cau)

extern volatile u8 D_801485B8; /* @source 0x801485B8 @kind unknown */
extern u8 D_801485B9; /* @source 0x801485B9 @kind unknown */
extern u8 D_801485BA; /* @source 0x801485BA @kind unknown */
extern s16 D_801485BC; /* @source 0x801485BC @kind unknown */
extern u16 D_801485BE; /* @source 0x801485BE @kind unknown */
extern u8 D_801485C2; /* @source 0x801485C2 @kind unknown */
extern u8 D_801485C3; /* @source 0x801485C3 @kind unknown */
extern u8 D_801485C4; /* @source 0x801485C4 @kind unknown */
extern u8 D_801485C5; /* @source 0x801485C5 @kind unknown */
extern u32 D_801485D8; /* @source 0x801485D8 @kind unknown */
extern u8 D_80148656; /* @source 0x80148656 @kind unknown */
extern u8 D_800B6F50[]; /* @source 0x800B6F50 @kind unknown */
extern u8 D_800B4EB0[]; /* @source 0x800B4EB0 @kind table */
extern volatile u8 D_801485DC; /* @source 0x801485DC @kind unknown */

/* @source 0x800B6D1C @kind unknown */
extern u8 battleApParamStrings;

/* @source 0x80096A04 @kind unknown */
extern u8 battlePanelIndexFormat;

/* @source 0x800969C4 @kind unknown */
extern u8 battleWorkStateHandlerTable;

extern u8 D_801462EC; /* @source 0x801462EC @kind unknown */
extern s16 D_8014930A; /* @source 0x8014930A @kind unknown */
extern s16 D_8014930E; /* @source 0x8014930E @kind unknown */
extern u8 D_80149332; /* @source 0x80149332 @kind unknown */
extern s8 D_800B44C0[]; /* @source 0x800B44C0 @kind table */

typedef struct BattlePartyFlagRecord {
  u32 flags_00;
  u8  pad_04[0x13C];
} BattlePartyFlagRecord;

extern BattlePartyFlagRecord D_80145FB8[]; /* @source 0x80145FB8 @kind unknown */
extern u8 D_80145F09[]; /* @source 0x80145F09 @kind unknown */
extern u8 D_800B6C40[]; /* @source 0x800B6C40 @kind unknown */
extern u8 D_800B6C4C[]; /* @source 0x800B6C4C @kind unknown */
extern u8 D_800B6C68[]; /* @source 0x800B6C68 @kind unknown */
extern u8 D_800B6C74[]; /* @source 0x800B6C74 @kind unknown */
extern u8 D_800E40CE[]; /* @source 0x800E40CE @kind unknown */
extern u8 D_801EB710[]; /* @source 0x801EB710 @kind unknown */

/* Enemy-work record (0x118 stride) flag word at 0x801EB734 (record + 0x08),
 * the +0x140 local-work counter byte at 0x80145FC9, the current action
 * pointer at 0x80146380 and the battle action dispatch tables. Referenced by
 * func_8009D198. */
typedef struct BattleEnemyWordRecord {
  u32 word_00;
  u8  pad_04[0x114];
} BattleEnemyWordRecord;

extern BattleEnemyWordRecord D_801EB734[]; /* @source 0x801EB734 @kind unknown */
extern u8  D_80145FC9[]; /* @source 0x80145FC9 @kind unknown */
extern u8  D_801EB745[]; /* @source 0x801EB745 @kind unknown */
extern u8* D_80146380; /* @source 0x80146380 @kind unknown */
extern u8  D_800B44F8[]; /* @source 0x800B44F8 @kind unknown */
extern u8* D_800B470C[]; /* @source 0x800B470C @kind unknown */
extern BattleSelectionHandler D_800B471C[]; /* @source 0x800B471C @kind unknown */

#endif

extern u8 D_80145F2F[]; /* @source 0x80145F2F @kind unknown */
extern u8 D_80145F30[]; /* @source 0x80145F30 @kind unknown */
extern u8 D_80145F31[]; /* @source 0x80145F31 @kind unknown */
extern u8 D_80145F32[]; /* @source 0x80145F32 @kind unknown */
extern u8 D_80145F33[]; /* @source 0x80145F33 @kind unknown */
extern u8 D_801EB6DF[]; /* @source 0x801EB6DF @kind unknown */
extern u8 D_801EB6E0[]; /* @source 0x801EB6E0 @kind unknown */
extern u8 D_801EB6E1[]; /* @source 0x801EB6E1 @kind unknown */
extern u8 D_801EB6E2[]; /* @source 0x801EB6E2 @kind unknown */
extern u8 D_801EB6E3[]; /* @source 0x801EB6E3 @kind unknown */
extern s16 battleElementResistanceModifierTable[]; /* @source 0x800B493C @kind table */

typedef struct BattleScanRecord {
  u8 kind;
  u8 unk_01[0x8B];
  u8 owner;
  u8 unk_8D[0x0B];
} BattleScanRecord;

extern BattleScanRecord D_8014688E[]; /* @source 0x8014688E @kind unknown */
