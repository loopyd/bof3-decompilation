#ifndef PLAYER_HUD_H
#define PLAYER_HUD_H

#include "audio.h"

void hud_draw(u32 frames, u32 ticks, const u8 amplitudes[AUDIO_CHANNELS]);

#endif
