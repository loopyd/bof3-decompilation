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
} BattleWork;

typedef struct BattleRange {
  u8  unk_00[0x34];
  u32 range_axis_34;
  u32 range_axis_38;
} BattleRange;

typedef struct BattleLocalWork {
  u8 unk_00;
  u8 unk_01;
  u8 unk_02;
  u8 unk_03;
  u8 unk_04;
  u8 unk_05[0x13B];
} BattleLocalWork;

typedef struct BattleLocalOffsetPair {
  s16 values[4][2];
} BattleLocalOffsetPair;

typedef struct Unk801EBF08 {
  u8  unk_00;
  u8  unk_01;
  u8  unk_02[0x126];
  volatile u32 unk_128;
} Unk801EBF08;


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
extern BattleSelectionHandler D_800B6BF4[]; /* @source 0x800B6BF4 @kind unknown */

u32 func_800AF66C(BattleRange *range, u32 value);

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
extern volatile u32  D_80146250; /* @source 0x80146250 @kind unknown */
extern volatile u8   D_801462E0; /* @source 0x801462E0 @kind unknown */
extern volatile u8   D_801462E1; /* @source 0x801462E1 @kind unknown */
extern volatile u8   D_801462E2; /* @source 0x801462E2 @kind unknown */
extern volatile u8   D_801462E3; /* @source 0x801462E3 @kind unknown */
extern BattleSelectionHandler D_800B43C0[]; /* @source 0x800B43C0 @kind unknown */
extern BattleSelectionHandler D_800B4450[]; /* @source 0x800B4450 @kind unknown */
extern BattleSelectionHandler D_800B43D4[]; /* @source 0x800B43D4 @kind unknown */
extern BattleSelectionHandler D_800B4CAC[]; /* @source 0x800B4CAC @kind unknown */
extern BattleSelectionHandler D_800B4CC8[]; /* @source 0x800B4CC8 @kind unknown */
extern BattleSelectionHandler D_800B4CD0[]; /* @source 0x800B4CD0 @kind unknown */
extern BattleSelectionHandler D_800B4CE4[]; /* @source 0x800B4CE4 @kind unknown */
extern BattleSelectionHandler D_800B4D00[]; /* @source 0x800B4D00 @kind unknown */
extern BattleSelectionHandler D_800B4D14[]; /* @source 0x800B4D14 @kind unknown */
extern BattleSelectionHandler D_800B4D30[]; /* @source 0x800B4D30 @kind unknown */
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
extern BattleSelectionHandler D_800B44C8[]; /* @source 0x800B44C8 @kind unknown */
extern BattleSelectionHandler D_800B44D4[]; /* @source 0x800B44D4 @kind unknown */
extern BattleSelectionHandler D_800B44E4[]; /* @source 0x800B44E4 @kind unknown */
extern BattleSelectionHandler D_800B6E08[]; /* @source 0x800B6E08 @kind unknown */
extern BattleLocalOffsetPair D_800B6C90[]; /* @source 0x800B6C90 @kind unknown */
extern u8 D_800B6D00[]; /* @source 0x800B6D00 @kind unknown */
extern volatile u8   D_801462E4; /* @source 0x801462E4 @kind unknown */
extern volatile u8   D_801462EF; /* @source 0x801462EF @kind unknown */
extern volatile u8   D_80146303; /* @source 0x80146303 @kind unknown */
extern volatile u16  D_80145AC8; /* @source 0x80145AC8 @kind unknown */
extern u8*           D_801EB4D8; /* @source 0x801EB4D8 @kind unknown */
extern u8*           D_801EBF08; /* @source 0x801EBF08 @kind unknown */
extern u8            D_80148330[]; /* @source 0x80148330 @kind unknown */
extern u8            D_801462E5; /* @source 0x801462E5 @kind unknown */
extern volatile u8   D_801462E6; /* @source 0x801462E6 @kind unknown */
extern BattleSelectionAction D_800B65FC[]; /* @source 0x800B65FC @kind unknown */
extern volatile u16  D_801462E8; /* @source 0x801462E8 @kind unknown */
extern volatile BattleLocalWork D_80145E90[]; /* @source 0x80145E90 @kind unknown */
extern u8  D_80145FB0[]; /* @source 0x80145FB0 @kind unknown */
extern u8  D_801EB2E8[]; /* @source 0x801EB2E8 @kind unknown */
extern u8  D_801EB72C[]; /* @source 0x801EB72C @kind unknown */
extern u8  D_801EC337[]; /* @source 0x801EC337 @kind unknown */
extern u8  D_801EC33B[]; /* @source 0x801EC33B @kind unknown */
extern u8  D_801EC357[]; /* @source 0x801EC357 @kind unknown */
extern u8  D_801EC390[]; /* @source 0x801EC390 @kind unknown */
extern u8  D_801EC3A4[]; /* @source 0x801EC3A4 @kind unknown */
extern volatile u8             D_80146329; /* @source 0x80146329 @kind unknown */
extern volatile u8             D_80146374; /* @source 0x80146374 @kind unknown */
extern volatile s8   D_80145558; /* @source 0x80145558 @kind unknown */
extern volatile s16  D_801EC2EE; /* @source 0x801EC2EE @kind unknown */
extern volatile u8   D_80146394; /* @source 0x80146394 @kind unknown */
extern s16* volatile D_801463A0; /* @source 0x801463A0 @kind unknown */
extern volatile u16  D_801463C0; /* @source 0x801463C0 @kind unknown */
extern u8            D_801463C9; /* @source 0x801463C9 @kind unknown */
extern volatile BattleSelectionKind D_801CA71C[]; /* @source 0x801CA71C @kind unknown */
extern volatile u8   D_8014837B; /* @source 0x8014837B @kind unknown */
extern volatile u8   D_8014839F; /* @source 0x8014839F @kind unknown */
extern volatile u8   D_801483C3; /* @source 0x801483C3 @kind unknown */
extern volatile u8   D_80148597; /* @source 0x80148597 @kind unknown */
extern volatile u8   D_801485BB; /* @source 0x801485BB @kind unknown */
extern volatile u8   D_801485DE; /* @source 0x801485DE @kind unknown */
extern volatile u8   D_801485DF; /* @source 0x801485DF @kind unknown */
extern volatile u16  D_801485E0; /* @source 0x801485E0 @kind unknown */
extern volatile u16  D_8014932E; /* @source 0x8014932E @kind unknown */
extern volatile u16  D_801485E2; /* @source 0x801485E2 @kind unknown */
extern volatile u16  D_801485EC; /* @source 0x801485EC @kind unknown */
extern volatile u16  D_801485EE; /* @source 0x801485EE @kind unknown */
extern volatile u8   D_80148626; /* @source 0x80148626 @kind unknown */
extern volatile u8   D_80148627; /* @source 0x80148627 @kind unknown */
extern volatile s16  D_80148628; /* @source 0x80148628 @kind unknown */
extern volatile u16  D_8014862A; /* @source 0x8014862A @kind unknown */
extern volatile u8   D_8014862E; /* @source 0x8014862E @kind unknown */

/* Shared primitive cursor (PsyQ SDK, owned by the main exe). */
extern u8* D_8014598C; /* @source 0x8014598C @kind unknown */

/* PsyQ SDK primitive setup helpers called by this target.
 * SetSprt8 / SetSemiTrans are declared by <libgpu.h> (via bof3/psyq.h);
 * func_8014E5A0 is a game primitive-append helper (lifted in exe/slus_004.22). */
void func_8014E5A0(u32 ot_index, u32 primitive_size);
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
void                           func_800A84FC(void);
void                           resetStateWhenUnlockedB(void);
void __attribute__((noinline)) func_8009B20C(void);
u8                             func_8009C8AC(u16 required_mask);
void                           func_8009CFEC(void);
s16  func_800A2880(u8 battler_index, u16 base_value, u8 element_flag);
s16  func_800A2AE0(u8 battler_index, u16 element_mask);
void func_800A31E0(u8 selection_kind, u16 input_mask);
u8   func_800A3A10(u8 battler_index, u8 selection_kind);
u16  func_800A36F0(u8 battler_index, u16 flags);
void func_800A3F28(void);
u8   resetSelectionApplyInput(s32 input_mask);
u8   func_801DB524(u8 arg0);
void func_800A4458(void);
void setupMode104ArmWorkBit2(void);
void func_800AAA74(void);
void func_800AAEBC(s16 target_index, u8 battler_index);
void func_800B0498(void);
void func_800B0B0C(s16 base_x, s16 base_y);
void func_800B2218(void);
void func_800B22AC(void);
void func_800B23B8(void);
void func_800B23F8(void);
void func_800B250C(void);
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
u32  func_801E590C(u32 arg0, u32 arg1);
void func_801DEA64(s32 arg0);

#define D_8014598C g_PrimCursor
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
extern volatile u8 D_801485DC; /* @source 0x801485DC @kind unknown */

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
extern s16 D_800B493C[]; /* @source 0x800B493C @kind unknown */
