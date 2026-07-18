#ifndef EMI_BATTLE_15_INTERNAL_H
#define EMI_BATTLE_15_INTERNAL_H

#include "bof3/bof3.h"
#include "bof3/ui/panel_task.h"

typedef void (*BattleSelectionHandler)(void);

typedef struct BattleLocalPanelEntry {
  u8  owner_index;
  u8  unk_01;
  u16 panel_id;
} BattleLocalPanelEntry;

extern vu8  BATTLE_SELECTION_PHASE;
extern vu8  BATTLE_SELECTION_OWNER_STATE;
extern vu8  BATTLE_SELECTION_ROOT_STATE;
extern vu8  BATTLE_SELECTION_SUBSTATE;
extern vu8  BATTLE_SELECTION_PENDING_KIND;
extern vu8  BATTLE_SELECTION_ADVANCE_COUNTER;
extern vu16 BATTLE_SELECTION_RING_RESET;
extern vu16 BATTLE_INPUT_HELD_MASK;
extern vu16 BATTLE_INPUT_CONFIRM_MASK;
extern vu16 BATTLE_INPUT_CANCEL_MASK;
extern vu8  BATTLE_PANEL_TASK_FLAG_A;
extern vu8  BATTLE_PANEL_TASK_FLAG_B;
extern vu8  BATTLE_PANEL_STATE_KIND;
#define BATTLE_ACTIVE_SELECTION_SLOT_PTR PTR_SLOT_AT(volatile u8, 0x801eb4d8u)
#define BATTLE_ACTIVE_MESSAGE_SLOT_PTR   PTR_SLOT_AT(volatile void, 0x801ebf08u)
#define BATTLE_CURRENT_BATTLER_PTR       PTR_SLOT_AT(volatile u8, 0x801eb4e8u)
extern vu8  BATTLE_SELECTION_LOCKED;
extern vu16 BATTLE_SELECTION_CURSOR_BASE_X;
extern vu16 BATTLE_SELECTION_CURSOR_BASE_Y;
extern vu8  BATTLE_SELECTION_MOVE_SFX;
extern vu8  BATTLE_SELECTION_RING_INDEX;
extern vu8  BATTLE_SELECTION_GROUP_INDEX;
extern vu8  BATTLE_SELECTION_CURSOR_INDEX;
extern s16  BATTLE_SELECTION_SCROLL_BASE;
extern s16  BATTLE_SELECTION_SCROLL_DELTA;
extern vu8  BATTLE_SELECTION_CURSOR_DIRTY;
extern vu16 BATTLE_SELECTION_CURSOR_X;
extern vu16 BATTLE_SELECTION_CURSOR_Y;
extern vu8  BATTLE_SELECTION_SOURCE_SLOT;
extern vu8  BATTLE_PANEL_ICON_RING_HEAD;
#define BATTLE_SELECTION_SLOT_SUBSTATE_TABLE \
  PTR_AT(const volatile BattleSelectionHandler, 0x800b43c0u)
#define BATTLE_SELECTION_CONFIRM_SUBSTATE_TABLE \
  PTR_AT(const volatile BattleSelectionHandler, 0x800b43ecu)
#define BATTLE_SELECTION_RESULT_SUBSTATE_TABLE \
  PTR_AT(const volatile BattleSelectionHandler, 0x800b43f4u)
#define BATTLE_SELECTION_FINALIZE_SUBSTATE_TABLE \
  PTR_AT(const volatile BattleSelectionHandler, 0x800b4408u)
#define BATTLE_SELECTION_SECONDARY_SUBSTATE_TABLE \
  PTR_AT(const volatile BattleSelectionHandler, 0x800b4450u)
#define BATTLE_SELECTION_RING_FLAG(index) \
  (*(volatile volatile u8*)(0x801eb5b1u + ((index) * 8u)))
#define BATTLE_SELECTION_RING_HANDLE(index) \
  (*(volatile volatile u32*)(0x801eb5b4u + ((index) * 8u)))
#define BATTLE_SELECTION_SAVED_GROUP(index) \
  (*(volatile volatile u8*)(0x801454f4u + ((index) * 3u)))
#define BATTLE_SELECTION_SAVED_SCROLL(index) \
  (*(volatile volatile u8*)(0x801454f5u + ((index) * 3u)))
#define BATTLE_SELECTION_SAVED_CURSOR(index) \
  (*(volatile volatile u8*)(0x801454f6u + ((index) * 3u)))
#define BATTLE_SELECTION_KIND_FLAGS(kind) \
  (*(volatile volatile u8*)(0x801ca718u + ((kind) * 0x14u)))
#define BATTLE_SELECTION_KIND_MASK(kind) \
  (*(volatile volatile u16*)(0x801ca71cu + ((kind) * 0x14u)))
#define BATTLE_SELECTION_KIND_NAME_ID(kind) \
  (*(volatile volatile u16*)(0x801ca71eu + ((kind) * 0x14u)))
extern vu8            BATTLE_SECONDARY_PANEL_ACTIVE;
extern vu8            BATTLE_SECONDARY_PANEL_ROWS;
extern vu8            BATTLE_SECONDARY_PANEL_KIND;
extern vu8            BATTLE_SECONDARY_STATE_KIND;
extern vu8            BATTLE_SECONDARY_SOURCE_GROUP;
extern vu8            BATTLE_SECONDARY_PAGE_BASE;
extern vu8            BATTLE_SECONDARY_CURSOR_INDEX;
extern vu8            BATTLE_SECONDARY_CURSOR_LIMIT;
extern vu16           BATTLE_SECONDARY_FLAG_MASK;
extern vu8            BATTLE_SELECTION_CURSOR_ROWS;
extern vu8            BATTLE_SELECTION_CURSOR_MODE;
extern Bof3PanelTask* D_80148648;
extern vu8            BATTLE_SECONDARY_SAVED_GROUP;
extern vu8            BATTLE_SECONDARY_SAVED_PAGE_BASE;
extern vu8            BATTLE_SECONDARY_SAVED_CURSOR;
extern vu16           BATTLE_SECONDARY_MOVE_SFX;
#define BATTLE_SECONDARY_GROUP_TABLE(index) \
  PTR_SLOT_AT(volatile u8, 0x801c893cu + ((index) * 4u))
#define BATTLE_SELECTION_PANEL_FLAGS(index) \
  (*(volatile volatile u32*)(0x80145fb4u + ((index) * 0x140u)))
extern vu8  BATTLE_SECONDARY_PROMPT_ACTIVE;
extern vu8  BATTLE_SECONDARY_PROMPT_ROWS;
extern vu8  BATTLE_SECONDARY_PROMPT_KIND;
extern vu16 BATTLE_SECONDARY_PROMPT_X;
extern vu16 BATTLE_SECONDARY_PROMPT_Y;
extern vu8  BATTLE_SECONDARY_PROMPT_MODE;
extern vu8  BATTLE_SECONDARY_PROMPT_CURSOR_LIMIT;
extern vu8  BATTLE_PANEL_PROMPT_STATE;
extern vu8  BATTLE_PANEL_RULE_PASS_KIND;
extern vu8  BATTLE_PANEL_RULE_PASS_SLOT;
extern vu16 BATTLE_PANEL_RULE_PASS_SELECTION;
#define BATTLE_LOCAL_PANEL_RULE(class_id, slot_index)            \
  PTR_AT(volatile u8, (0x800e407cu + ((u32)(class_id) * 0x88u) + \
                       ((u32)(slot_index) * 0x10u)))
#define BATTLE_PANEL_SLOT_KIND(slot_index) \
  (*(volatile volatile u8*)(0x80145f12u + ((u32)(slot_index) * 0x140u)))
#define BATTLE_PANEL_SLOT_MASK(kind) \
  (*(volatile volatile u8*)(0x801c90ebu + ((u32)(kind) * 0x18u)))
extern vu8 BATTLE_LOCAL_PANEL_ENTRY_COUNT;
#define BATTLE_LOCAL_PANEL_OWNER_KIND(index) \
  (*(volatile volatile u8*)(0x801eb6acu + ((u32)(index) * 0x118u)))
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

void                           func_800975D4(void);
void                           func_800989B4(void);
void __attribute__((noinline)) func_8009B20C(void);
u8                             func_8009C8AC(u16 required_mask);
void                           func_8009CFEC(void);

s16  func_800A2880(u8 battler_index, u16 base_value, u8 element_flag);
s16  func_800A2AE0(u8 battler_index, u16 element_mask);
u16  func_800A36F0(u8 battler_index, u16 flags);
void func_800A4458(void);
void func_800AAA74(void);
void func_800AAEBC(s16 target_index, u8 battler_index);
void func_800B0498(void);
void func_800B0B0C(s16 base_x, s16 base_y);
void func_800B2218(void);
void func_800B22AC(void);
void func_800B23F8(void);
void func_800B250C(void);

#endif
