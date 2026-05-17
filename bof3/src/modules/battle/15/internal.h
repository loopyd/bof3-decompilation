#ifndef BOF3_SRC_MODULES_BATTLE_15_INTERNAL_H
#define BOF3_SRC_MODULES_BATTLE_15_INTERNAL_H

#include "bof3/context.h"

typedef void (*BattleSelectionHandler)(void);

typedef struct BattleLocalPanelEntry {
  u8  owner_index;
  u8  unk_01;
  u16 panel_id;
} BattleLocalPanelEntry;

#define BATTLE_SELECTION_PHASE           (*(volatile u8*)0x801462e1u)
#define BATTLE_SELECTION_OWNER_STATE     (*(volatile u8*)0x801462e2u)
#define BATTLE_SELECTION_ROOT_STATE      (*(volatile u8*)0x801462e3u)
#define BATTLE_SELECTION_SUBSTATE        (*(volatile u8*)0x801462e4u)
#define BATTLE_SELECTION_PENDING_KIND    (*(volatile u8*)0x801462efu)
#define BATTLE_SELECTION_ADVANCE_COUNTER (*(volatile u8*)0x80146303u)
#define BATTLE_SELECTION_RING_RESET      (*(volatile u16*)0x80145ac8u)
#define BATTLE_INPUT_HELD_MASK           (*(volatile u16*)0x80145aa8u)
#define BATTLE_INPUT_CONFIRM_MASK        (*(volatile u16*)0x80145ac2u)
#define BATTLE_INPUT_CANCEL_MASK         (*(volatile u16*)0x80145ac4u)
#define BATTLE_PANEL_TASK_FLAG_A         (*(volatile u8*)0x8014837bu)
#define BATTLE_PANEL_TASK_FLAG_B         (*(volatile u8*)0x8014839fu)
#define BATTLE_PANEL_STATE_KIND          (*(volatile u8*)0x801483c3u)
#define BATTLE_ACTIVE_SELECTION_SLOT_PTR (*(volatile u8**)0x801eb4d8u)
#define BATTLE_ACTIVE_MESSAGE_SLOT_PTR   (*(volatile void**)0x801ebf08u)
#define BATTLE_CURRENT_BATTLER_PTR       (*(volatile u8**)0x801eb4e8u)
#define BATTLE_SELECTION_LOCKED          (*(volatile u8*)0x80148573u)
#define BATTLE_SELECTION_CURSOR_BASE_X   (*(volatile u16*)0x80148574u)
#define BATTLE_SELECTION_CURSOR_BASE_Y   (*(volatile u16*)0x80148576u)
#define BATTLE_SELECTION_MOVE_SFX        (*(volatile u8*)0x80148579u)
#define BATTLE_SELECTION_RING_INDEX      (*(volatile u8*)0x8014857au)
#define BATTLE_SELECTION_GROUP_INDEX     (*(volatile u8*)0x8014857bu)
#define BATTLE_SELECTION_CURSOR_INDEX    (*(volatile u8*)0x8014857cu)
#define BATTLE_SELECTION_SCROLL_BASE     (*(volatile s16*)0x80148580u)
#define BATTLE_SELECTION_SCROLL_DELTA    (*(volatile s16*)0x80148582u)
#define BATTLE_SELECTION_CURSOR_DIRTY    (*(volatile u8*)0x801485dcu)
#define BATTLE_SELECTION_CURSOR_X        (*(volatile u16*)0x801485e0u)
#define BATTLE_SELECTION_CURSOR_Y        (*(volatile u16*)0x801485e2u)
#define BATTLE_SELECTION_SOURCE_SLOT     (*(volatile u8*)0x80148656u)
#define BATTLE_PANEL_ICON_RING_HEAD      (*(volatile u8*)0x801ec328u)
#define BATTLE_SELECTION_SLOT_SUBSTATE_TABLE \
  ((BattleSelectionHandler const volatile*)0x800b43c0u)
#define BATTLE_SELECTION_CONFIRM_SUBSTATE_TABLE \
  ((BattleSelectionHandler const volatile*)0x800b43ecu)
#define BATTLE_SELECTION_RESULT_SUBSTATE_TABLE \
  ((BattleSelectionHandler const volatile*)0x800b43f4u)
#define BATTLE_SELECTION_FINALIZE_SUBSTATE_TABLE \
  ((BattleSelectionHandler const volatile*)0x800b4408u)
#define BATTLE_SELECTION_SECONDARY_SUBSTATE_TABLE \
  ((BattleSelectionHandler const volatile*)0x800b4450u)
#define BATTLE_SELECTION_RING_FLAG(index) \
  (*(volatile u8*)(0x801eb5b1u + ((index) * 8u)))
#define BATTLE_SELECTION_RING_HANDLE(index) \
  (*(volatile u32*)(0x801eb5b4u + ((index) * 8u)))
#define BATTLE_SELECTION_SAVED_GROUP(index) \
  (*(volatile u8*)(0x801454f4u + ((index) * 3u)))
#define BATTLE_SELECTION_SAVED_SCROLL(index) \
  (*(volatile u8*)(0x801454f5u + ((index) * 3u)))
#define BATTLE_SELECTION_SAVED_CURSOR(index) \
  (*(volatile u8*)(0x801454f6u + ((index) * 3u)))
#define BATTLE_SELECTION_KIND_FLAGS(kind) \
  (*(volatile u8*)(0x801ca718u + ((kind) * 0x14u)))
#define BATTLE_SELECTION_KIND_MASK(kind) \
  (*(volatile u16*)(0x801ca71cu + ((kind) * 0x14u)))
#define BATTLE_SELECTION_KIND_NAME_ID(kind) \
  (*(volatile u16*)(0x801ca71eu + ((kind) * 0x14u)))
#define BATTLE_SECONDARY_PANEL_ACTIVE    (*(volatile u8*)0x80148570u)
#define BATTLE_SECONDARY_PANEL_ROWS      (*(volatile u8*)0x80148571u)
#define BATTLE_SECONDARY_PANEL_KIND      (*(volatile u8*)0x80148572u)
#define BATTLE_SECONDARY_STATE_KIND      (*(volatile u8*)0x80148578u)
#define BATTLE_SECONDARY_SOURCE_GROUP    (*(volatile u8*)0x8014857au)
#define BATTLE_SECONDARY_PAGE_BASE       (*(volatile u8*)0x8014857bu)
#define BATTLE_SECONDARY_CURSOR_INDEX    (*(volatile u8*)0x8014857cu)
#define BATTLE_SECONDARY_CURSOR_LIMIT    (*(volatile u8*)0x8014857du)
#define BATTLE_SECONDARY_FLAG_MASK       (*(volatile u16*)0x80148584u)
#define BATTLE_SELECTION_CURSOR_ROWS     (*(volatile u8*)0x801485ddu)
#define BATTLE_SELECTION_CURSOR_MODE     (*(volatile u8*)0x801485deu)
#define BATTLE_LOCAL_PANEL_TASK_ROOT     (*(volatile u8**)0x80148648u)
#define BATTLE_SECONDARY_SAVED_GROUP     (*(volatile u8*)0x801454fdu)
#define BATTLE_SECONDARY_SAVED_PAGE_BASE (*(volatile u8*)0x801454feu)
#define BATTLE_SECONDARY_SAVED_CURSOR    (*(volatile u8*)0x801454ffu)
#define BATTLE_SECONDARY_MOVE_SFX        (*(volatile u16*)0x80148580u)
#define BATTLE_SECONDARY_GROUP_TABLE(index) \
  (*(volatile u8**)(0x801c893cu + ((index) * 4u)))
#define BATTLE_SELECTION_PANEL_FLAGS(index) \
  (*(volatile u32*)(0x80145fb4u + ((index) * 0x140u)))
#define BATTLE_SECONDARY_PROMPT_ACTIVE       (*(volatile u8*)0x80148624u)
#define BATTLE_SECONDARY_PROMPT_ROWS         (*(volatile u8*)0x80148625u)
#define BATTLE_SECONDARY_PROMPT_KIND         (*(volatile u8*)0x80148626u)
#define BATTLE_SECONDARY_PROMPT_X            (*(volatile u16*)0x80148628u)
#define BATTLE_SECONDARY_PROMPT_Y            (*(volatile u16*)0x8014862au)
#define BATTLE_SECONDARY_PROMPT_MODE         (*(volatile u8*)0x8014862eu)
#define BATTLE_SECONDARY_PROMPT_CURSOR_LIMIT (*(volatile u8*)0x8014862fu)
#define BATTLE_PANEL_PROMPT_STATE            (*(volatile u8*)0x801483c0u)
#define BATTLE_PANEL_RULE_PASS_KIND          (*(volatile u8*)0x80146375u)
#define BATTLE_PANEL_RULE_PASS_SLOT          (*(volatile u8*)0x80146374u)
#define BATTLE_PANEL_RULE_PASS_SELECTION     (*(volatile u16*)0x801463c0u)
#define BATTLE_LOCAL_PANEL_RULE(class_id, slot_index)  \
  ((volatile u8*)(0x800e407cu + ((u32)(class_id) * 0x88u) + \
                  ((u32)(slot_index) * 0x10u)))
#define BATTLE_PANEL_SLOT_KIND(slot_index) \
  (*(volatile u8*)(0x80145f12u + ((u32)(slot_index) * 0x140u)))
#define BATTLE_PANEL_SLOT_MASK(kind) \
  (*(volatile u8*)(0x801c90ebu + ((u32)(kind) * 0x18u)))
#define BATTLE_LOCAL_PANEL_ENTRY_COUNT (*(volatile u8*)0x801eb5a8u)
#define BATTLE_LOCAL_PANEL_OWNER_KIND(index) \
  (*(volatile u8*)(0x801eb6acu + ((u32)(index) * 0x118u)))
#define BATTLE_LOCAL_PANEL_ENTRY(index) \
  (*(volatile BattleLocalPanelEntry*)(0x801ed9b0u + ((u32)(index) * 4u)))

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

void __attribute__((noinline)) func_8009b20c(void);
u8                             func_8009c8ac(u16 required_mask);
void                           func_8009cfec(void);

s16  func_800a2880(u8 battler_index, u16 base_value, u8 element_flag);
s16  func_800a2ae0(u8 battler_index, u16 element_mask);
u16  func_800a36f0(u8 battler_index, u16 flags);
void func_800a4458(void);
void func_800aaa74(void);
void func_800aaebc(s16 target_index, u8 battler_index);
void func_800b0498(void);
void func_800b0b0c(s16 base_x, s16 base_y);

#endif
