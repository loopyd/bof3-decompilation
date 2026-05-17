#ifndef BOF3_CONTEXT_01_PROTOTYPES_H
#define BOF3_CONTEXT_01_PROTOTYPES_H

/* function prototypes */

void game_front_local_mode_callback_loop(void);
void game_set_frontend_layout_bank(u32 layout_bank);
void game_start_selection_fx(u32 effect_group, s32 effect_id, s32 duration, s32 fade_step);
void game_stop_selection_fx(u32 effect_group, s32 effect_id);
void emi_stream_init_slot(u32 slot_id);
void game_set_active_selection_cue(u8 selection_id, s32 cue_level, s32 cue_shape);
void game_stage_shared_palette_bank(void);
void game_queue_frontend_cue(u32 cue_id);
#endif
