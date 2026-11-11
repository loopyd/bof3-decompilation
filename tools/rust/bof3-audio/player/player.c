#include "audio.h"
#include "controls.h"
#include "graphics.h"
#include "hud.h"
#include "config.h"
#include "sfx.h"
#include "vocal.h"

void player_main(void)
{
    u32 first_frame, buttons;
#if PLAYER_MODE_MUSIC
    u32 ticks;
    u8 amplitudes[AUDIO_CHANNELS];
#endif
    graphics_initialize();
#if PLAYER_MODE_SFX
    sfx_initialize();
#elif PLAYER_MODE_VOCAL
    vocal_initialize();
#else
    audio_initialize();
#endif
    controls_initialize();
    first_frame = (u32)CALL(0x80174700, -1, 0, 0, 0);
    player_status.phase = 4;
    for (;;) {
        CALL(0x80174700, 0, 0, 0, 0);
        player_status.frames = (u32)CALL(0x80174700, -1, 0, 0, 0) - first_frame;
#if PLAYER_MODE_SFX
        sfx_update();
        buttons = controls_read();
        if (buttons & BUTTON_LEFT) sfx_select_bank(-1);
        else if (buttons & BUTTON_RIGHT) sfx_select_bank(1);
        if (buttons & BUTTON_UP) sfx_select_sample(-1);
        else if (buttons & BUTTON_DOWN) sfx_select_sample(1);
        if (buttons & BUTTON_CROSS) sfx_play();
        if (buttons & BUTTON_SQUARE) sfx_halt();
#elif PLAYER_MODE_VOCAL
        buttons = controls_read();
        if (buttons & BUTTON_CIRCLE) vocal_stop();
        else if (buttons & BUTTON_CROSS) vocal_play();
        if (buttons & BUTTON_SQUARE) vocal_repeat();
        vocal_update();
#else
        audio_update();
        buttons = controls_read();
        if (buttons & BUTTON_UP) audio_select(-1);
        else if (buttons & BUTTON_DOWN) audio_select(1);
        if (buttons & BUTTON_CROSS) audio_pause();
        if (buttons & BUTTON_SQUARE) audio_restart();
        if (buttons & BUTTON_LEFT) audio_seek(-1);
        else if (buttons & BUTTON_RIGHT) audio_seek(1);
        ticks = audio_ticks();
        audio_levels(amplitudes);
#endif
        graphics_begin_frame(player_status.frames);
        if (player_controls.help) hud_help();
        else {
#if PLAYER_MODE_SFX
            hud_sfx();
#elif PLAYER_MODE_VOCAL
            hud_vocal();
#else
            hud_draw(player_status.position_frames, ticks, amplitudes);
#endif
        }
        graphics_present();
    }
}
