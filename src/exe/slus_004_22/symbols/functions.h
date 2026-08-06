#ifndef SLUS_004_22_SYMBOLS_FUNCTIONS_H
#define SLUS_004_22_SYMBOLS_FUNCTIONS_H

#include "bof3/bof3.h"

typedef void (*GameCallbackEntry)(void);

/* Runtime equipment-record dispatch. */
void* equip_record_base(s32 item_type, s32 item_index);

/* LOGO.EXE is loaded independently; this call targets its reviewed entry
 * address rather than linking LOGO.EXE implementation into SLUS_004.22. */
void func_801CE758(void);

/* SLUS startup, callback scheduler, and executable-file loading. */
void boot_noop(void);
void boot_main_loop(void);
void boot_init_runtime(void);
void boot_init_disc_events(void);
void boot_init_display_envs(void);
void boot_clear_ot_entry(u8* work);
void boot_capture_vsync(void);
void boot_run_logo_exe(void);
void func_8014AFC0(void);
void game_dispatch_callback_slots(void);
void func_8014B6B4(void);
void func_8014E22C(void);
s32  func_8014E0FC(const char* path);
void render_clear_rect(s16 x, s16 y, s16 width, s16 height);
void func_8014E6D0(void);
void func_8014EA80(void);
void func_8015CEBC(void);
void func_8015D044(void);

/* Newly discovered SLUS services — pending decompilation. */
void game_slot_scheduler_yield(u16 countdown);
void render_append_prim(u32 ot_index, u32 primitive_size);
void game_fade_loop(s32 a0, s32 a1, s32 a2);
u8   game_fade_draw_tile(s16* value, s32 arg, u8 arg2, u8 arg3, u8 arg4);
void func_80150098(s16 x, s16 y, u32 clut, const u8* text);
void sound_dispatch_cue(u32 cue_id);
s32  func_801655F4(u8* counter, s32 delta);

/* EMI loader and CD callback path. */
void    emi_loader_initialize(void);
DiscLba emi_loader_slot_lba(EmiLoaderSlotId slot_id);
void    emi_loader_begin_transfer(void);
void    emi_cd_sync_callback(s32 status, u8* result);
void    emi_cd_ready_callback(u8 status, u8* result);
void    emi_loader_validate_header(void);
void    emi_copy_type0_payload(void);
void    emi_record_dispatch_handler(void);
void    emi_record_packed_dispatch(void);
void    emi_select_next_entry(void);
void    emi_start_entry_transfer(void);
void    emi_select_primary_destination(void);
void    emi_select_alternate_destination(void);
s32     emi_stage_transfer_slot(u8 slot);
void    emi_copy_transfer_chunk(void);
void    emi_loader_select_mode6(void);
void    emi_dispatch_mode_callback(void);
void    func_80163010(void);

/* Remaining reviewed SLUS services, kept address-traceable pending promotion. */
void func_8016728C(u8 index, u8 family);
void SpuSetTransferMode(s32 arg);
void func_8016AD2C(s32 owner);
void SsUtAllKeyOff(s32 arg);
void SsVabClose(s32 resource_id);
s16  SsVabOpenHeadSticky(u32 source, s32 resource_id, u32 destination);
void func_8017B8D4(void* arg0, s32 arg1);
void func_8017BA40(void* arg0);
void func_8017BC98(void* arg0);
/* Semantic aliases preserve address-based names for analyzer and matching
 * tools while making reviewed call sites readable. */

#endif
