#include "config.h"
#include "sfx.h"
#include "audio.h"

#if PLAYER_MODE_SFX
#include "banks.h"

typedef short (*KeyOn)(short, short, short, short, short, short, short, short);
static u32 last_frame;
static u32 voice_started;

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

void sfx_update(void)
{
    u32 now, delta, envelope;
    now = (u32)CALL(0x80174700, -1, 0, 0, 0);
    delta = now - last_frame;
    last_frame = now;
    if (player_status.playing) {
        player_status.position_frames += delta;
        envelope = HALF(0x1f801c0c) & 0x7fff;
        if (envelope != 0) voice_started = 1;
        if (envelope == 0 && (voice_started || (HALF(0x1f801d9c) & 1))) {
            player_status.playing = 0;
        }
    }
}

void sfx_halt(void)
{
    CALL(0x8016e794, 0, 0, 0, 0);
    /* Flush the queued key-off before a caller may replace this bank. */
    CALL(0x8016c548, 0, 0, 0, 0);
    player_status.playing = 0;
    player_status.playing_sample = 0;
}

void sfx_play(void)
{
    const struct SfxSample *sample;
    int result;
    sample = sfx_sample();
    if (!sample->resolved) return;
    sfx_halt();
    result = ((KeyOn)0x8016e400)(0, (short)sfx_bank()->asset.bank,
        (short)sample->program, (short)sample->tone, (short)sample->key, 0, 127, 127);
    CALL(0x8016c548, 0, 0, 0, 0);
    player_status.error = result == 0 ? 0 : result;
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
    u32 bank, frames;
    bank = player_status.bank;
    if (direction < 0 && bank != 0) --bank;
    if (direction > 0 && bank + 1 < PLAYER_BANK_COUNT) ++bank;
    if (bank == player_status.bank) return;
    sfx_halt();
    /* Do not replace sample RAM while the old voice still has a release tail. */
    for (frames = 0; (HALF(0x1f801c0c) & 0x7fff) != 0; ++frames) {
        if (frames == 600) {
            player_status.error = -5;
            return;
        }
        CALL(0x80174700, 0, 0, 0, 0);
    }
    CALL(0x80173818, sfx_bank()->asset.bank, 0, 0, 0);
    player_status.bank = bank;
    player_status.sample = 0;
    player_status.position_frames = 0;
    player_status.error = 0;
    bank_load(&sfx_bank()->asset);
    player_status.phase = 4;
}
#endif
