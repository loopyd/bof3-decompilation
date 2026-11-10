#include "audio.h"
#include "controls.h"
#include "graphics.h"
#include "hud.h"

void player_main(void)
{
    u32 first_frame, ticks, buttons;
    u8 amplitudes[AUDIO_CHANNELS];
    graphics_initialize();
    audio_initialize();
    controls_initialize();
    first_frame = (u32)CALL(0x80174700, -1, 0, 0, 0);
    player_status.phase = 4;
    for (;;) {
        CALL(0x80174700, 0, 0, 0, 0);
        player_status.frames = (u32)CALL(0x80174700, -1, 0, 0, 0) - first_frame;
        audio_update();
        buttons = controls_read();
        if (buttons & BUTTON_UP) audio_select(-1);
        else if (buttons & BUTTON_DOWN) audio_select(1);
        if (buttons & BUTTON_CROSS) audio_pause();
        if (buttons & BUTTON_SQUARE) audio_restart();
        ticks = audio_ticks();
        audio_levels(amplitudes);
        graphics_begin_frame(player_status.frames);
        if (player_controls.help) hud_help();
        else hud_draw(player_status.position_frames, ticks, amplitudes);
        graphics_present();
    }
}
