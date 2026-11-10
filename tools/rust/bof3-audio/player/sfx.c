#include "config.h"
#include "sfx.h"
#include "audio.h"

#if PLAYER_MODE_SFX
#include "banks.h"

typedef short (*KeyOn)(short, short, short, short, short, short, short, short);
typedef void (*DispatchCue)(u32);
static u32 last_frame;
static u32 voice_started;
static u32 voice_mask;

const struct SfxBank *sfx_bank(void)
{
    return &player_banks[player_status.bank];
}

const struct SfxSample *sfx_sample(void)
{
    return &sfx_bank()->samples[player_status.sample];
}

void sfx_initialize(void)
{
    player_status.phase = 1;
    CALL(0x8015cd00, 0, 0, 0, 0);
    bank_load(&sfx_bank()->asset);
    /* SsStart flushes SDK voice caches without opening a SEP. */
    CALL(0x8016c498, 0, 0, 0, 0);
    last_frame = (u32)CALL(0x80174700, -1, 0, 0, 0);
}

static int voices_sounding(void)
{
    u32 voice;
    for (voice = 0; voice < 24; ++voice) {
        if ((voice_mask & (1u << voice)) && (HALF(0x1f801c0c + 16 * voice) & 0x7fff)) {
            return 1;
        }
    }
    return 0;
}

void sfx_update(void)
{
    u32 now, delta, ended;
    now = (u32)CALL(0x80174700, -1, 0, 0, 0);
    delta = now - last_frame;
    last_frame = now;
    if (player_status.playing) {
        player_status.position_frames += delta;
        if (voices_sounding()) voice_started = 1;
        else {
            ended = HALF(0x1f801d9c) | ((u32)HALF(0x1f801d9e) << 16);
            if (voice_started || (ended & voice_mask) == voice_mask) {
                player_status.playing = 0;
            }
        }
    }
}

void sfx_halt(void)
{
    u32 voice;
    if (voice_mask) {
        for (voice = 0; voice < 24; ++voice) {
            if (voice_mask & (1u << voice)) CALL(0x8016e794, voice, 0, 0, 0);
        }
        CALL(0x8016c548, 0, 0, 0, 0);
    }
    player_status.playing = 0;
    player_status.playing_sample = 0;
}

static int release_voices(void)
{
    u32 frames;
    sfx_halt();
    for (frames = 0; voices_sounding(); ++frames) {
        if (frames == 600) {
            player_status.error = -5;
            return 0;
        }
        CALL(0x80174700, 0, 0, 0, 0);
    }
    voice_mask = 0;
    return 1;
}

static int play_game_cue(u32 cue)
{
    u32 layers, layer, voice, actual_mask;
    CALL(0x8015d044, 0, 0, 0, 0);
    ((DispatchCue)0x8015df18)(cue);
    layers = HALF(0x8018b3e4);
    if (layers > 3) return -6;
    actual_mask = 0;
    for (layer = 0; layer <= layers; ++layer) {
        voice = HALF(0x8018b318 + layer * 0x28);
        if (voice >= 24 || HALF(0x8018b31c + layer * 0x28) != voice) return -6;
        actual_mask |= 1u << voice;
    }
    return actual_mask == voice_mask ? 0 : -6;
}

void sfx_play(void)
{
    const struct SfxSample *sample;
    int result;
    sample = sfx_sample();
    if (sample->voices == 0 || !release_voices()) return;
    voice_mask = sample->voices;
    if (sample->playback == SFX_GAME_CUE) result = play_game_cue(sample->cue);
    else result = ((KeyOn)0x8016e400)(0, (short)sfx_bank()->asset.bank,
        (short)sample->program, (short)sample->tone, (short)sample->key, 0, 127, 127);
    CALL(0x8016c548, 0, 0, 0, 0);
    if (result != 0) sfx_halt();
    player_status.error = result;
    player_status.playing = result == 0;
    player_status.playing_sample = result == 0 ? player_status.sample + 1 : 0;
    player_status.position_frames = 0;
    last_frame = (u32)CALL(0x80174700, -1, 0, 0, 0);
    voice_started = 0;
}

void sfx_select_sample(int direction)
{
    u32 sample;
    sample = player_status.sample;
    if (direction < 0 && sample != 0) --sample;
    if (direction > 0 && sample + 1 < sfx_bank()->sample_count) ++sample;
    player_status.sample = sample;
}

void sfx_select_bank(int direction)
{
    u32 bank;
    bank = player_status.bank;
    if (direction < 0 && bank != 0) --bank;
    if (direction > 0 && bank + 1 < PLAYER_BANK_COUNT) ++bank;
    if (bank == player_status.bank) return;
    if (!release_voices()) return;
    CALL(0x80173818, sfx_bank()->asset.bank, 0, 0, 0);
    player_status.bank = bank;
    player_status.sample = 0;
    player_status.position_frames = 0;
    player_status.error = 0;
    bank_load(&sfx_bank()->asset);
    player_status.phase = 4;
}
#endif
