#include "config.h"
#include "audio.h"
#include "bank.h"

extern const u8 player_header[], player_body[], player_sequence[];
typedef int (*NoteCall)(u32, u32, u32, u32, u32, u32);
static u32 player_record;
#if PLAYER_MODE_MUSIC
#define SEEK_FRAMES (5u * 60u)
#define SPU_VOICES 0x1f801c00
static int sequence_handle;
static int pending_start;
static int rewind_pending;
static int muted;
static u32 last_frame;
static u32 music_frame;
static u32 seek_target;
static u32 seek_ticks;
static u32 track_ends[PLAYER_SEQUENCE_COUNT];
#endif
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

#if PLAYER_MODE_MUSIC
static void clear_voice_tracks(void)
{
    u32 voice;
    for (voice = 0; voice < 24; ++voice) voice_tracks[voice] = 255;
}

static u32 sequence_ticks(void)
{
    u32 end, delay;
    /* The delta reader counts ten units per MIDI tick. Subtract its
     * unconsumed delay to obtain the current position. */
    end = WORD(player_record + 0x80);
    delay = WORD(player_record + 0x88);
    if ((int)delay > 0 && delay <= end) end -= delay;
    return end / 10;
}

static void play_track(void)
{
    CALL(0x8016b9cc, sequence_handle, player_status.track, 1, 1);
    pending_start = 0;
}

static void set_muted(int mute)
{
    int volume;
    if (muted == mute) return;
    volume = mute ? 0 : 127;
    CALL(0x8016c0c0, volume, volume, 0, 0);
    muted = mute;
}

static void reset_seek_audio(void)
{
    u32 voice, registers;
    CALL(0x8016d6c4, sequence_handle, player_status.track, 0, 0);
    /* Replay the original SPU startup voice reset with its resident silent
     * block. The music player owns all 24 voices and keeps the bank loaded. */
    for (voice = 0; voice < 24; ++voice) {
        registers = SPU_VOICES + voice * 16;
        HALF(registers) = 0;
        HALF(registers + 2) = 0;
        HALF(registers + 4) = 0x3fff;
        HALF(registers + 6) = 0x200;
        HALF(registers + 8) = 0;
        HALF(registers + 10) = 0;
        voice_tracks[voice] = 255;
    }
    HALF(0x1f801d88) = 0xffff;
    HALF(0x1f801d8a) = 0xff;
    CALL(0x8016f514, -1, 0, 0, 0);
    CALL(0x80170680, 0, 0, 0, 0);
    music_frame = 0;
    seek_ticks = 0;
    rewind_pending = 1;
}

void audio_initialize(void)
{
    static const struct BankAsset asset = {
        PLAYER_LAYOUT, PLAYER_BANK, PLAYER_SPU_BASE, PLAYER_HEADER_ADDRESS,
        PLAYER_HEADER_BYTES, PLAYER_BODY_BYTES, 0, player_header, player_body, 0, 0
    };
    int handle;
    clear_voice_tracks();
    player_status.phase = 1;
    CALL(0x8015cd00, 0, 0, 0, 0);
    bank_load(&asset);
    bank_copy((u8 *)PLAYER_SEQUENCE_ADDRESS, player_sequence, PLAYER_SEQUENCE_BYTES);
    handle = CALL(0x8016b38c, PLAYER_SEQUENCE_ADDRESS, PLAYER_BANK, PLAYER_SEQUENCE_COUNT, 0);
    if (handle < 0 || handle >= 2) bank_fail(handle);
    CALL(0x8015ce70, 0, 0, 0, 0);
    player_record = WORD(0x80190308 + handle * 4) + PLAYER_SEQUENCE_INDEX * 0xac;
    sequence_handle = handle;
    player_status.track = PLAYER_SEQUENCE_INDEX;
    play_track();
}

void audio_update(void)
{
    u32 delta;
    delta = player_status.frames - last_frame;
    last_frame = player_status.frames;
    if (rewind_pending) {
        HALF(0x1f801d8c) = 0xffff;
        HALF(0x1f801d8e) = 0xff;
        rewind_pending = 0;
        if (seek_target == 0 && player_status.paused) {
            pending_start = 1;
            player_status.position_frames = 0;
            player_status.seeking = 0;
            return;
        }
        play_track();
        delta = 0;
    }
    if (!player_status.seeking && (player_status.paused || player_status.ended)) return;
    music_frame += delta;
    if ((WORD(player_record + 0x90) & 1) == 0) {
        track_ends[player_status.track] = music_frame;
        if (player_status.seeking) {
            player_status.ended = 1;
            /* The original Stop clears the record's elapsed ticks. */
            player_status.midi_ticks = seek_ticks;
        } else {
            music_frame = 0;
            play_track();
            ++player_status.restarts;
        }
    } else if (player_status.seeking) seek_ticks = sequence_ticks();
    if (player_status.seeking) {
        if (!player_status.ended) {
            if (music_frame < seek_target) return;
            /* A learned endpoint can move by a VBlank on a new play. */
            if (seek_target != 0 && seek_target == track_ends[player_status.track]) return;
            if (player_status.paused) CALL(0x8016b968, sequence_handle, player_status.track, 0, 0);
            else set_muted(0);
        }
        player_status.seeking = 0;
    }
    player_status.position_frames = music_frame;
}

void audio_pause(void)
{
    if (player_status.seeking) {
        player_status.paused = !player_status.paused;
        return;
    }
    if (player_status.ended) {
        player_status.paused = 0;
        audio_restart();
        return;
    }
    if (!player_status.paused) {
        CALL(0x8016b968, sequence_handle, player_status.track, 0, 0);
        player_status.paused = 1;
        return;
    }
    if (pending_start) play_track();
    else CALL(0x8016bb6c, sequence_handle, player_status.track, 0, 0);
    player_status.paused = 0;
    set_muted(0);
}

void audio_restart(void)
{
    music_frame = 0;
    player_status.position_frames = 0;
    player_status.midi_ticks = 0;
    player_status.ended = 0;
    pending_start = player_status.paused != 0;
    if (player_status.seeking || muted) {
        reset_seek_audio();
        seek_target = 0;
        player_status.seeking = 1;
    } else {
        CALL(0x8016d6c4, sequence_handle, player_status.track, 0, 0);
        if (!pending_start) play_track();
    }
    ++player_status.restarts;
}

void audio_select(int direction)
{
    u32 track;
    track = player_status.track;
    if (direction < 0 && track != 0) --track;
    if (direction > 0 && track + 1 < PLAYER_SEQUENCE_COUNT) ++track;
    if (track == player_status.track) return;
    CALL(0x8016d6c4, sequence_handle, player_status.track, 0, 0);
    clear_voice_tracks();
    player_status.track = track;
    player_record = WORD(0x80190308 + sequence_handle * 4) + track * 0xac;
    audio_restart();
}

void audio_seek(int direction)
{
    u32 origin, target, end;
    origin = player_status.seeking && !player_status.paused ?
        music_frame : player_status.position_frames;
    end = track_ends[player_status.track];
    if (direction < 0) target = origin > SEEK_FRAMES ? origin - SEEK_FRAMES : 0;
    else {
        target = origin + SEEK_FRAMES;
        if (target < origin) target = ~0u;
        if (end != 0 && target > end) target = end;
    }
    if (!player_status.seeking && target == origin) return;
    if (!player_status.seeking) seek_ticks = sequence_ticks();
    set_muted(1);
    if (target < music_frame || (player_status.paused && !player_status.seeking) ||
        player_status.ended) reset_seek_audio();
    seek_target = target;
    player_status.ended = 0;
    player_status.seeking = 1;
}

u32 audio_ticks(void)
{
    if (!player_status.seeking && !player_status.ended) player_status.midi_ticks = sequence_ticks();
    return player_status.midi_ticks;
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
#endif
