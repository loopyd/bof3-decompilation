#include "config.h"
#include "audio.h"

extern const u8 player_header[], player_body[], player_sequence[];
typedef int (*NoteCall)(u32, u32, u32, u32, u32, u32);
static u32 player_record;
static int sequence_handle;
static u8 voice_tracks[24];
volatile struct PlayerStatus player_status;

/* The original SEP note caller supplies a voice mask, but the SPU has no MIDI
 * channel register. Record that association around the unchanged note routine. */
int player_note_on(u32 id, u32 bank, u32 program, u32 note, u32 velocity, u32 pan)
{
    u32 record, track, voice;
    int mask;
    record = WORD(0x80190308 + (id & 255) * 4) + (id >> 8) * 0xac;
    track = BYTE(record + 0x12);
    mask = ((NoteCall)0x8017102c)(id, bank, program, note, velocity, pan);
    if (mask != -1 && record == player_record && track < AUDIO_CHANNELS) {
        for (voice = 0; voice < 24; ++voice) {
            if ((u32)mask & (1u << voice)) voice_tracks[voice] = (u8)track;
        }
    }
    return mask;
}

static void copy(u8 *destination, const u8 *source, u32 bytes)
{
    while (bytes-- != 0) *destination++ = *source++;
}

static void fail(int error)
{
    player_status.error = error;
    player_status.phase = 255;
    for (;;) CALL(0x80174700, 0, 0, 0, 0);
}

void audio_initialize(void)
{
    u32 offset, bytes;
    int result, handle;
    for (offset = 0; offset < 24; ++offset) voice_tracks[offset] = 255;
    player_status.phase = 1;
    CALL(0x8015cd00, 0, 0, 0, 0);
    CALL(0x80161808, PLAYER_LAYOUT, 0, 0, 0);
    if (PLAYER_SPU_BASE + ((PLAYER_BODY_BYTES + 63) & ~63u) >
        (u32)HALF(0x1f801da2) * 8) fail(-4);
    copy((u8 *)PLAYER_HEADER_ADDRESS, player_header, PLAYER_HEADER_BYTES);
    copy((u8 *)PLAYER_SEQUENCE_ADDRESS, player_sequence, PLAYER_SEQUENCE_BYTES);
    player_status.phase = 2;
    result = CALL(0x80173c50, PLAYER_HEADER_ADDRESS, PLAYER_BANK, PLAYER_SPU_BASE, 0);
    if (result != PLAYER_BANK) fail(result);
    for (offset = 0; offset < PLAYER_BODY_BYTES; offset += bytes) {
        bytes = PLAYER_BODY_BYTES - offset;
        if (bytes > 2048) bytes = 2048;
        result = CALL(0x80174354, player_body + offset, bytes, PLAYER_BANK, 0);
        if (result != (offset + bytes == PLAYER_BODY_BYTES ? PLAYER_BANK : -2)) fail(result);
        if (CALL(0x80174598, 1, 0, 0, 0) != 1) fail(-3);
    }
    player_status.phase = 3;
    handle = CALL(0x8016b38c, PLAYER_SEQUENCE_ADDRESS, PLAYER_BANK, PLAYER_SEQUENCE_COUNT, 0);
    if (handle < 0 || handle >= 2) fail(handle);
    CALL(0x8015ce70, 0, 0, 0, 0);
    player_record = WORD(0x80190308 + handle * 4) + PLAYER_SEQUENCE_INDEX * 0xac;
    sequence_handle = handle;
    CALL(0x8016b9cc, handle, PLAYER_SEQUENCE_INDEX, 1, 1);
}

void audio_update(void)
{
    if ((WORD(player_record + 0x90) & 1) == 0) {
        CALL(0x8016b9cc, sequence_handle, PLAYER_SEQUENCE_INDEX, 1, 1);
        ++player_status.restarts;
    }
}

u32 audio_ticks(void)
{
    u32 end, delay, ticks;
    /* The original delta reader adds ten units per MIDI tick at +0x80.
     * +0x88 holds the unconsumed delay, including tempo scheduling units. */
    end = WORD(player_record + 0x80);
    delay = WORD(player_record + 0x88);
    ticks = end / 10;
    if ((int)delay > 0 && delay <= end) ticks = (end - delay) / 10;
    player_status.midi_ticks = ticks;
    return ticks;
}

void audio_levels(u8 current[AUDIO_CHANNELS])
{
    u32 voice, track, level, volume, envelope;
    int left, right;
    for (track = 0; track < AUDIO_CHANNELS; ++track) current[track] = 0;
    for (voice = 0; voice < 24; ++voice) {
        track = voice_tracks[voice];
        if (track >= AUDIO_CHANNELS) continue;
        envelope = HALF(0x1f801c0c + voice * 16) & 0x7fff;
        left = HALF(0x1f801c00 + voice * 16) & 0x7fff;
        right = HALF(0x1f801c02 + voice * 16) & 0x7fff;
        if (left & 0x4000) left -= 0x8000;
        if (right & 0x4000) right -= 0x8000;
        if (left < 0) left = -left;
        if (right < 0) right = -right;
        volume = (u32)(left > right ? left : right);
        level = volume * envelope >> 22;
        if (level > current[track]) current[track] = (u8)level;
    }
}
