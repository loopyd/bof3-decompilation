#ifndef BOF3_CONTEXT_15_PROTOTYPES_H
#define BOF3_CONTEXT_15_PROTOTYPES_H

/* function prototypes */

void battle_stage_attack_name_message(s32 slot_index, s32 queue_kind);
u8   battle_resolve_selection_slot(u32 family_id);
void battle_queue_frontend_cue(u32 cue_id);
u32  battle_resolve_frontend_resource(u16 resource_id);
void battle_stage_selection_ring_record(u32 slot_index, u32 record_kind, u32 resource_handle);
u32  battle_decode_repeatable_input(u16 input_mask);
u8   battle_selection_kind_is_blocked(void);
void battle_reset_local_task_slot(void);
void battle_stage_message_resource(void* message_slot);
u8   battle_result_uses_empty_slot(void);
u8   battle_local_panel_slot_has_entry(volatile u8* battler, u32 slot_index);
void battle_copy_local_panel_rule_entry(volatile u8* battler, volatile u8* panel_rule);
void battle_set_local_panel_slot_active(volatile u8* battler, u32 slot_index, u32 active_state);
u16  battle_resolve_secondary_choice_resource(u32 group_index, u32 choice_id);
u8   battle_try_commit_secondary_choice(u32 panel_kind, u32 zero_arg, u32 group_index, u32 choice_id);
void __attribute__((noinline)) func_8009b20c(void);
u8                             func_8009c8ac(u16 required_mask);
void                           func_8009cfec(void);
#endif
