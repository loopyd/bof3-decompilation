#ifndef EMI_BATTLE_15_INTERNAL_H
#define EMI_BATTLE_15_INTERNAL_H

#include "bof3/bof3.h"
#include "bof3/ui/panel_task.h"
#include "panel/task.h"
#include "gpu/prim.h"
#include "battle/ram.h"

typedef void (*BattleSelectionHandler)(void);

typedef struct BattleLocalPanelEntry {
  u8  owner_index;
  u8  unk_01;
  u16 panel_id;
} BattleLocalPanelEntry;

extern volatile u8  BATTLE_SELECTION_PHASE;
extern volatile u8  BATTLE_SELECTION_OWNER_STATE;
extern volatile u8  BATTLE_SELECTION_ROOT_STATE;
extern volatile u8  BATTLE_SELECTION_SUBSTATE;
extern volatile u8  BATTLE_SELECTION_PENDING_KIND;
extern volatile u8  BATTLE_SELECTION_ADVANCE_COUNTER;
extern volatile u16 BATTLE_SELECTION_RING_RESET;
extern volatile u16 BATTLE_INPUT_HELD_MASK;
extern volatile u16 BATTLE_INPUT_CONFIRM_MASK;
extern volatile u16 BATTLE_INPUT_CANCEL_MASK;
extern volatile u8  BATTLE_PANEL_TASK_FLAG_A;
extern volatile u8  BATTLE_PANEL_TASK_FLAG_B;
extern volatile u8  BATTLE_PANEL_STATE_KIND;
extern volatile u8  BATTLE_SELECTION_LOCKED;
extern volatile u16 BATTLE_SELECTION_CURSOR_BASE_X;
extern volatile u16 BATTLE_SELECTION_CURSOR_BASE_Y;
extern volatile u8  BATTLE_SELECTION_MOVE_SFX;
extern volatile u8  BATTLE_SELECTION_RING_INDEX;
extern volatile u8  BATTLE_SELECTION_GROUP_INDEX;
extern volatile u8  BATTLE_SELECTION_CURSOR_INDEX;
extern s16          BATTLE_SELECTION_SCROLL_BASE;
extern s16          BATTLE_SELECTION_SCROLL_DELTA;
extern volatile u8  BATTLE_SELECTION_CURSOR_DIRTY;
extern volatile u16 BATTLE_SELECTION_CURSOR_X;
extern volatile u16 BATTLE_SELECTION_CURSOR_Y;
extern volatile u8  BATTLE_SELECTION_SOURCE_SLOT;
extern volatile u8  BATTLE_PANEL_ICON_RING_HEAD;
extern volatile u8  BATTLE_SECONDARY_PANEL_ACTIVE;
extern volatile u8  BATTLE_SECONDARY_PANEL_ROWS;
extern volatile u8  BATTLE_SECONDARY_PANEL_KIND;
extern volatile u8  BATTLE_SECONDARY_STATE_KIND;
extern volatile u8  BATTLE_SECONDARY_SOURCE_GROUP;
extern volatile u8  BATTLE_SECONDARY_PAGE_BASE;
extern volatile u8  BATTLE_SECONDARY_CURSOR_INDEX;
extern volatile u8  BATTLE_SECONDARY_CURSOR_LIMIT;
extern volatile u16 BATTLE_SECONDARY_FLAG_MASK;
extern volatile u8  BATTLE_SELECTION_CURSOR_ROWS;
extern volatile u8  BATTLE_SELECTION_CURSOR_MODE;
extern PanelTask*   D_80148648;
extern volatile u8  BATTLE_SECONDARY_SAVED_GROUP;
extern volatile u8  BATTLE_SECONDARY_SAVED_PAGE_BASE;
extern volatile u8  BATTLE_SECONDARY_SAVED_CURSOR;
extern volatile u16 BATTLE_SECONDARY_MOVE_SFX;
extern volatile u8  BATTLE_SECONDARY_PROMPT_ACTIVE;
extern volatile u8  BATTLE_SECONDARY_PROMPT_ROWS;
extern volatile u8  BATTLE_SECONDARY_PROMPT_KIND;
extern volatile u16 BATTLE_SECONDARY_PROMPT_X;
extern volatile u16 BATTLE_SECONDARY_PROMPT_Y;
extern volatile u8  BATTLE_SECONDARY_PROMPT_MODE;
extern volatile u8  BATTLE_SECONDARY_PROMPT_CURSOR_LIMIT;
extern volatile u8  BATTLE_PANEL_PROMPT_STATE;
extern volatile u8  BATTLE_PANEL_RULE_PASS_KIND;
extern volatile u8  BATTLE_PANEL_RULE_PASS_SLOT;
extern volatile u16 BATTLE_PANEL_RULE_PASS_SELECTION;
extern volatile u8 BATTLE_LOCAL_PANEL_ENTRY_COUNT;

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

void                           func_800975D4(void);
void                           func_800989B4(void);
void __attribute__((noinline)) func_8009B20C(void);
u8                             func_8009C8AC(u16 required_mask);
void                           func_8009CFEC(void);

s16  func_800A2880(u8 battler_index, u16 base_value, u8 element_flag);
s16  func_800A2AE0(u8 battler_index, u16 element_mask);
void func_800A31E0(u8 selection_kind, u16 input_mask);
u8   func_800A3A10(u8 battler_index, u8 selection_kind);
u16  func_800A36F0(u8 battler_index, u16 flags);
void func_800A3F28(void);
u8   func_800A41D8(s32 input_mask);
u8   func_801DB524(u8 arg0);
void func_800A4458(void);
void func_800AAA74(void);
void func_800AAEBC(s16 target_index, u8 battler_index);
void func_800B0498(void);
void func_800B0B0C(s16 base_x, s16 base_y);
void func_800B2218(void);
void func_800B22AC(void);
void func_800B23B8(void);
void func_800B23F8(void);
void func_800B250C(void);
u8   func_800A4238(s32 input_mask);
s16  func_801DC044(u8 arg0, u8 arg1, u32 arg2);
void func_801647C4(u16 arg0, u16 arg1, s32 arg2);
void func_801DE94C(s32 arg0, s32 arg1);
void func_80158E20(void);
u32  func_801502D0(u32 arg0);
void func_801DE8C0(u8 arg0, u8 arg1, u32 arg2);
u8   func_801DB5CC(s32 arg0);
void func_801E5988(void);

/* Scratchpad work-area pointer (volatile cell at 0x1F800044).
 * Reloaded per access to match original codegen. */
extern u8* volatile g_battle_work;

/* Absolute-address globals. Bound via WEAK_SYMBOL_AT in symbols.c; values
 * equal the symbol-name addresses. */
extern volatile u32  D_80144F60[];
extern volatile u32  D_80144F80[];
extern volatile u32  D_80145FAA;
extern volatile u32  D_80146250;
extern volatile u8   D_801462E3;
extern BattleSelectionHandler D_800B43D4[];
extern BattleSelectionHandler D_800B4CAC[];
extern BattleSelectionHandler D_800B4418[];
extern BattleSelectionHandler D_800B44A0[];
extern BattleSelectionHandler D_800B4428[];
extern BattleSelectionHandler D_800B4458[];
extern BattleSelectionHandler D_800B446C[];
extern BattleSelectionHandler D_800B447C[];
extern BattleSelectionHandler D_800B448C[];
extern BattleSelectionHandler D_800B44C8[];
extern BattleSelectionHandler D_800B44D4[];
extern BattleSelectionHandler D_800B44E4[];
extern volatile u8   D_801462E4;
extern volatile u8   D_801462EF;
extern volatile u16  D_80145AC8;
extern u8            D_80148330[];
extern volatile u32  D_801462E5;
extern volatile u32  D_801462E6;
extern volatile u32  D_801462E8;
extern volatile u8   D_80146374;
extern volatile u8   D_80146394;
extern volatile u32  D_801463A0;
extern volatile u32  D_801483C3;
extern volatile u32  D_80148597;
extern volatile u32  D_801485BB;
extern volatile u32  D_801485DF;
extern volatile u32  D_801485E0;
extern volatile u32  D_80148627;
extern volatile u32  D_8014862E;

/* Shared primitive cursor (PsyQ SDK, owned by the main exe). */
extern u8* D_8014598C;

/* PsyQ SDK primitive setup helpers called by this target.
 * SetSprt8 / SetSemiTrans are declared by <libgpu.h> (via bof3/psyq.h);
 * func_8014E5A0 is a game primitive-append helper (lifted in exe/slus_004.22). */
void func_8014E5A0(u32 ot_index, u32 primitive_size);

#define D_8014598C g_PrimCursor
#define BATTLE_ACTIVE_SELECTION_SLOT_PTR PSX_REF(volatile u8*, 0x801eb4d8u)
#define BATTLE_ACTIVE_MESSAGE_SLOT_PTR   PSX_REF(volatile void*, 0x801ebf08u)
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
#define BATTLE_PALETTE_TABLE       PSX_PTR(volatile u16, 0x800b6d30u)
#define BATTLE_PALETTE_ROW_28      PSX_PTR(volatile void, 0x800b6d28u)
#define BATTLE_PALETTE_ROW_20      PSX_PTR(volatile void, 0x800b6d20u)
#define BATTLE_UNK_800B6D1C        PSX_PTR(volatile u32, 0x800b6d1cu)
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

#endif
