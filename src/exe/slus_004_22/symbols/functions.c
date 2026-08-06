#include "bof3/context.h"

/* SLUS startup, callback scheduler, and executable-file loading. */
WEAK_SYMBOL_AT(boot_noop, 0x8014aa04);
WEAK_SYMBOL_AT(boot_init_display_envs, 0x8014ae08);
WEAK_SYMBOL_AT(boot_clear_ot_entry, 0x8014ae9c);
WEAK_SYMBOL_AT(boot_run_logo_exe, 0x8014aee0);
WEAK_SYMBOL_AT(func_8014AFC0, 0x8014afc0);
WEAK_SYMBOL_AT(func_8014B020, 0x8014b020);
WEAK_SYMBOL_AT(render_link_ot_packets, 0x8014b0f0);
WEAK_SYMBOL_AT(boot_capture_vsync, 0x8014b17c);
WEAK_SYMBOL_AT(game_dispatch_callback_slots, 0x8014b33c);
WEAK_SYMBOL_AT(func_8014B6B4, 0x8014b6b4);
WEAK_SYMBOL_AT(game_slot_scheduler_tick, 0x8014b73c);
WEAK_SYMBOL_AT(game_install_callback_slot, 0x8014b854);
WEAK_SYMBOL_AT(func_8014E22C, 0x8014e22c);
WEAK_SYMBOL_AT(func_8014E6D0, 0x8014e6d0);
WEAK_SYMBOL_AT(func_8014EA80, 0x8014ea80);
WEAK_SYMBOL_AT(game_front_local_mode_callback_loop, 0x8014ed6c);
WEAK_SYMBOL_AT(func_8015CEBC, 0x8015cebc);
WEAK_SYMBOL_AT(func_8015D044, 0x8015d044);

/* EMI loader and CD callback path. */
WEAK_SYMBOL_AT(emi_cd_sync_callback, 0x801621e8);
WEAK_SYMBOL_AT(emi_cd_ready_callback, 0x80162230);
WEAK_SYMBOL_AT(emi_loader_is_ready, 0x80162d00);
WEAK_SYMBOL_AT(func_80163010, 0x80163010);

/* Newly discovered SLUS services — pending decompilation. */
WEAK_SYMBOL_AT(game_slot_scheduler_yield, 0x8014b87c);
WEAK_SYMBOL_AT(render_append_prim, 0x8014e5a0);
WEAK_SYMBOL_AT(game_fade_loop, 0x8014f514);
WEAK_SYMBOL_AT(game_fade_draw_tile, 0x8014f704);
WEAK_SYMBOL_AT(func_80150098, 0x80150098);
WEAK_SYMBOL_AT(sound_dispatch_cue, 0x8015df18);
WEAK_SYMBOL_AT(func_801655F4, 0x801655f4);

/* Remaining reviewed SLUS services, kept address-traceable pending promotion. */
