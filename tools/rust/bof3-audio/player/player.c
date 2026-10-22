#include "audio.h"
#include "graphics.h"
#include "hud.h"

void player_main(void)
{
    u32 first_frame, ticks;
    u8 amplitudes[AUDIO_CHANNELS];
    graphics_initialize();
    audio_initialize();
    first_frame = (u32)CALL(0x80174700, -1, 0, 0, 0);
    player_status.phase = 4;
    for (;;) {
        CALL(0x80174700, 0, 0, 0, 0);
        player_status.frames = (u32)CALL(0x80174700, -1, 0, 0, 0) - first_frame;
        ticks = audio_ticks();
        audio_levels(amplitudes);
        graphics_begin_frame(player_status.frames);
        hud_draw(player_status.frames, ticks, amplitudes);
        graphics_present();
        audio_update();
    }
}
