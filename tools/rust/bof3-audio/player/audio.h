#ifndef PLAYER_AUDIO_H
#define PLAYER_AUDIO_H

#include "psx.h"

#define AUDIO_CHANNELS 16

struct PlayerStatus {
    u32 phase;
    u32 frames;
    u32 restarts;
    int error;
    u32 midi_ticks;
};
extern volatile struct PlayerStatus player_status;

void audio_initialize(void);
void audio_update(void);
u32 audio_ticks(void);
void audio_levels(u8 amplitudes[AUDIO_CHANNELS]);

#endif
