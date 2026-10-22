#include "config.h"

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef int (*Call)(u32, u32, u32, u32);
#define CALL(address, a, b, c, d) (((Call)(address))((u32)(a), (u32)(b), (u32)(c), (u32)(d)))
#define HALF(address) (*(volatile u16 *)(address))
#define BYTE(address) (*(volatile u8 *)(address))
#define WORD(address) (*(volatile u32 *)(address))
extern const u8 player_header[], player_body[], player_sequence[];
extern const u32 player_font[], player_palette[], player_logo[];
typedef int (*NoteCall)(u32, u32, u32, u32, u32, u32);
static u32 player_record;
static u8 voice_tracks[24];
static u8 levels[16];
static u32 glyph_packets[1024];
static u32 glyph_head;
static u32 draw_y;
struct PlayerStatus {
    u32 phase;
    u32 frames;
    u32 restarts;
    int error;
    u32 midi_ticks;
};
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
    if (mask != -1 && record == player_record && track < 16) {
        for (voice = 0; voice < 24; ++voice) {
            if ((u32)mask & (1u << voice)) voice_tracks[voice] = (u8)track;
        }
    }
    return mask;
}

static void gpu(u32 command)
{
    while ((WORD(0x1f801814) & 0x04000000) == 0) {}
    WORD(0x1f801810) = command;
}

static u32 xy(u32 x, u32 y) { return x | (y << 16); }

static void upload(u32 x, u32 y, u32 width, u32 height, const u32 *pixels)
{
    u32 count;
    gpu(0xa0000000);
    gpu(xy(x, y));
    gpu(xy(width, height));
    count = width * height / 2;
    while (count-- != 0) gpu(*pixels++);
}

static void rectangle(u32 x, u32 y, u32 width, u32 height, u32 color)
{
    gpu(0x60000000 | color);
    gpu(xy(x, y));
    gpu(xy(width, height));
}

static void text_scaled(u32 x, u32 y, const char *string, u32 width, u32 height)
{
    u32 *packet, *end, length, index, px, py, uv, u, v;
    if (width == 0 || height == 0) return;
    glyph_head = 0x00ffffff;
    WORD(0x8014598c) = (u32)glyph_packets;
    WORD(0x801459d4) = (u32)&glyph_head;
    BYTE(0x80143d44) = 12;
    CALL(0x80150098, 0, 0, 0, string);
    end = (u32 *)WORD(0x8014598c);
    for (packet = glyph_packets; packet < end; packet += length + 1) {
        length = *packet >> 24;
        if (length == 3) {
            px = x + (packet[2] & 0xffff) * width / 8;
            py = y + (packet[2] >> 16) * height / 8;
            if (width == 8 && height == 8) {
                gpu(packet[1]); gpu(xy(px, py)); gpu(packet[3]);
            } else {
                uv = packet[3]; u = uv & 255; v = (uv >> 8) & 255;
                gpu(0x2e000000 | (packet[1] & 0xffffff));
                gpu(xy(px, py)); gpu(uv);
                gpu(xy(px + width, py)); gpu((u + 8) | (v << 8) | 0x002f0000);
                gpu(xy(px, py + height)); gpu(u | ((v + 8) << 8));
                gpu(xy(px + width, py + height)); gpu((u + 8) | ((v + 8) << 8));
            }
        } else {
            for (index = 1; index <= length; ++index) gpu(packet[index]);
        }
    }
}

static void text(u32 x, u32 y, const char *string)
{
    text_scaled(x, y, string, 8, 8);
}

static void display_initialize(void)
{
    CALL(0x8017af0c, 0, 0, 0, 0);
    WORD(0x1f801814) = 0x08000001;
    WORD(0x1f801814) = 0x06c60260;
    WORD(0x1f801814) = 0x07042018;
    WORD(0x1f801814) = 0x03000000;
    gpu(0xe6000000);
    upload(960, 0, 64, 256, player_font);
    upload(0, 480, 256, 1, player_palette);
    upload(512, 0, 160, 80, player_logo);
}

static void logo(void)
{
    gpu(0xe1000108);
    gpu(0x65808080);
    gpu(xy(12, 4));
    gpu(0);
    gpu(xy(160, 80));
}

static void timecode(u32 frames)
{
    char digits[12];
    u32 seconds, minutes, smpte;
    /* 30-frame non-drop timecode: one timecode frame per two NTSC VBlanks. */
    smpte = frames / 2;
    seconds = smpte / 30;
    minutes = seconds / 60;
    digits[0] = (char)('0' + minutes / 60 / 10 % 10);
    digits[1] = (char)('0' + minutes / 60 % 10);
    digits[2] = ':';
    digits[3] = (char)('0' + minutes / 10 % 6);
    digits[4] = (char)('0' + minutes % 10);
    digits[5] = ':';
    digits[6] = (char)('0' + seconds / 10 % 6);
    digits[7] = (char)('0' + seconds % 10);
    digits[8] = ':';
    digits[9] = (char)('0' + smpte % 30 / 10);
    digits[10] = (char)('0' + smpte % 10);
    digits[11] = 0;
    text_scaled(184, 54, "SMPTE 30 NDF", 6, 6);
    text(184, 64, digits);
}

static void decimal(char *destination, u32 value, u32 digits)
{
    while (digits != 0) {
        destination[--digits] = (char)('0' + value % 10);
        value /= 10;
    }
}

static void musical_position(void)
{
    u32 end, delay, ticks, quarter, denominator, beats, remainder;
    char digits[16];
    /* The original delta reader adds ten units per MIDI tick at +0x80.
     * +0x88 holds the unconsumed delay, including tempo scheduling units. */
    end = WORD(player_record + 0x80);
    delay = WORD(player_record + 0x88);
    ticks = end / 10;
    if ((int)delay > 0 && delay <= end) ticks = (end - delay) / 10;
    player_status.midi_ticks = ticks;
    quarter = PLAYER_RESOLUTION * 4;
    denominator = 1u << PLAYER_DENOMINATOR_POWER;
    remainder = ticks % quarter * denominator;
    beats = ticks / quarter * denominator + remainder / quarter;
    decimal(digits, beats / PLAYER_NUMERATOR + 1, 4);
    digits[4] = ':';
    decimal(digits + 5, beats % PLAYER_NUMERATOR + 1, 3);
    digits[8] = ':';
    decimal(digits + 9, remainder % quarter / denominator, 6);
    digits[15] = 0;
    text_scaled(184, 76, "BAR:BEAT:TICK", 6, 6);
    text_scaled(184, 84, digits, 6, 6);
}

static void meters(void)
{
    u32 voice, track, level, volume, envelope, x, bar, color;
    int left, right;
    u8 current[16];
    char number[3];
    for (track = 0; track < 16; ++track) current[track] = 0;
    for (voice = 0; voice < 24; ++voice) {
        track = voice_tracks[voice];
        if (track >= 16) continue;
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
    for (track = 0; track < 16; ++track) {
        level = current[track];
        if (level >= levels[track]) levels[track] = (u8)level;
        else levels[track] = levels[track] > 4 ? levels[track] - 4 : 0;
        x = 17 + track * 18;
        color = 0x181818 + ((level * 200 / 127) << 8) + ((level * 100 / 127) << 16);
        rectangle(x + 2, 118, 7, 7, color);
        for (bar = 0; bar < 16; ++bar) {
            color = 0x292820;
            if (levels[track] > bar * 8) color = bar < 11 ? 0xa0d030 : (bar < 14 ? 0x30ccdc : 0x4050f0);
            rectangle(x + 1, 204 - bar * 4, 9, 3, color);
        }
        number[0] = (char)('0' + (track + 1) / 10);
        number[1] = (char)('0' + (track + 1) % 10);
        number[2] = 0;
        text(x - 2, 216, number);
    }
}

static void display(u32 frames)
{
    u32 next;
    next = draw_y == 0 ? 240 : 0;
    gpu(0xe3000000 | (next << 10));
    gpu(0xe4000000 | 319 | ((next + 239) << 10));
    gpu(0xe5000000 | (next << 11));
    rectangle(0, 0, 320, 240, 0x201810);
    logo();
    text(184, 22, "PLAYER " PLAYER_VERSION);
    text(184, 42, "deitydragon");
    timecode(frames);
    musical_position();
    text(16, 96, PLAYER_SONG);
    meters();
    while ((WORD(0x1f801814) & 0x04000000) == 0) {}
    WORD(0x1f801814) = 0x05000000 | (next << 10);
    draw_y = next;
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

void player_main(void)
{
    u32 offset, bytes, record, first_frame;
    int result, handle;
    display_initialize();
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
    record = WORD(0x80190308 + handle * 4) + PLAYER_SEQUENCE_INDEX * 0xac;
    player_record = record;
    CALL(0x8016b9cc, handle, PLAYER_SEQUENCE_INDEX, 1, 1);
    first_frame = (u32)CALL(0x80174700, -1, 0, 0, 0);
    player_status.phase = 4;
    for (;;) {
        CALL(0x80174700, 0, 0, 0, 0);
        player_status.frames = (u32)CALL(0x80174700, -1, 0, 0, 0) - first_frame;
        display(player_status.frames);
        if ((WORD(record + 0x90) & 1) == 0) {
            CALL(0x8016b9cc, handle, PLAYER_SEQUENCE_INDEX, 1, 1);
            ++player_status.restarts;
        }
    }
}
