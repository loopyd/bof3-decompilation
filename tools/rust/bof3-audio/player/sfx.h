#ifndef PLAYER_SFX_H
#define PLAYER_SFX_H

#include "bank.h"

enum SfxPlayback { SFX_UNRESOLVED, SFX_SDK_NOTE, SFX_GAME_CUE, SFX_AMBIGUOUS };

struct SfxSample {
    enum SfxPlayback playback;
    u32 program, tone, key, cue, voices;
};
struct SfxBank {
    struct BankAsset asset;
    u32 sample_count;
    const struct SfxSample *samples;
    const char *name;
};

void sfx_initialize(void);
void sfx_update(void);
void sfx_select_bank(int direction);
void sfx_select_sample(int direction);
void sfx_play(void);
void sfx_halt(void);
const struct SfxBank *sfx_bank(void);
const struct SfxSample *sfx_sample(void);

#endif
