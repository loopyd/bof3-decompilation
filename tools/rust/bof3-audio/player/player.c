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
static u8 peaks[16];
static u8 peak_hold[16];
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

static int sine(u32 angle)
{
    static const int quarter[17] = {
        0, 100, 200, 297, 392, 483, 569, 650, 724,
        792, 851, 903, 946, 980, 1004, 1019, 1024
    };
    u32 index, fraction;
    int value;
    angle &= 255;
    index = angle & 127;
    if (index > 64) index = 128 - index;
    fraction = index & 3;
    index >>= 2;
    value = quarter[index];
    if (index < 16) value += (quarter[index + 1] - value) * fraction / 4;
    return angle & 128 ? -value : value;
}

static void background(u32 frames)
{
    static const u8 faces[6][4] = {
        {0, 1, 2, 3}, {4, 5, 6, 7}, {0, 1, 4, 5},
        {2, 3, 6, 7}, {0, 2, 4, 6}, {1, 3, 5, 7}
    };
    u32 points[8], vertex, face, corner, axis;
    int x, y, z, rotated_x, rotated_y, rotated_z, yaw_sin, yaw_cos;
    int pitch_sin, pitch_cos;
    gpu(0xe1000200);
    gpu(0x38000000); gpu(xy(0, 0));
    gpu(0); gpu(xy(320, 0));
    gpu(0x380624); gpu(xy(0, 240));
    gpu(0x380624); gpu(xy(320, 240));
    yaw_sin = sine(frames / 3);
    yaw_cos = sine(frames / 3 + 64);
    pitch_sin = sine(frames / 5 + 24);
    pitch_cos = sine(frames / 5 + 88);
    for (vertex = 0; vertex < 8; ++vertex) {
        x = vertex & 1 ? 64 : -64;
        y = vertex & 2 ? 64 : -64;
        z = vertex & 4 ? 64 : -64;
        rotated_x = (x * yaw_cos + z * yaw_sin) / 1024;
        rotated_z = (z * yaw_cos - x * yaw_sin) / 1024;
        rotated_y = (y * pitch_cos - rotated_z * pitch_sin) / 1024;
        rotated_z = (y * pitch_sin + rotated_z * pitch_cos) / 1024;
        points[vertex] = xy(160 + rotated_x * 300 / (rotated_z + 320),
                            148 + rotated_y * 300 / (rotated_z + 320));
    }
    /* Quarter-additive faces and edges keep the cube below the HUD's contrast. */
    gpu(0xe1000260);
    for (face = 0; face < 6; ++face) {
        gpu(0x2a402030);
        for (corner = 0; corner < 4; ++corner) gpu(points[faces[face][corner]]);
    }
    for (vertex = 0; vertex < 8; ++vertex) {
        for (axis = 1; axis <= 4; axis <<= 1) {
            if ((vertex & axis) == 0) {
                gpu(0x42903068);
                gpu(points[vertex]); gpu(points[vertex | axis]);
            }
        }
    }
    gpu(0xe1000200);
    gpu(0x62000000); gpu(xy(176, 16)); gpu(xy(132, 78));
    gpu(0x62000000); gpu(xy(10, 92)); gpu(xy(300, 18));
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

static u32 meter_level(u32 amplitude)
{
    u32 base, level;
    if (amplitude == 0) return 0;
    base = 1;
    level = 1;
    while (amplitude >= base * 2) {
        base *= 2;
        level += 18;
    }
    return level + (amplitude - base) * 18 / base;
}

static void meters(void)
{
    static const u32 fire[16] = {
        0x080c90, 0x080ea8, 0x0814c0, 0x081dd8,
        0x0828f0, 0x0838ff, 0x084aff, 0x085eff,
        0x0874ff, 0x088cff, 0x08a4ff, 0x10baff,
        0x20ceff, 0x38deff, 0x58ecff, 0x80f8ff
    };
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
    gpu(0xe1000200);
    for (track = 0; track < 16; ++track) {
        level = meter_level(current[track]);
        if (level >= levels[track]) levels[track] = (u8)level;
        else levels[track] = levels[track] > 4 ? levels[track] - 4 : 0;
        if (levels[track] >= peaks[track]) {
            peaks[track] = levels[track];
            peak_hold[track] = 24;
        } else if (peak_hold[track] != 0) --peak_hold[track];
        else if (peaks[track] != 0) --peaks[track];
        x = 17 + track * 18;
        gpu(0x62000000); gpu(xy(x - 1, 116)); gpu(xy(13, 95));
        rectangle(x - 1, 116, 1, 95, 0x100610);
        rectangle(x + 11, 116, 1, 95, 0x100610);
        color = level != 0 ? fire[level / 8] : 0x100818;
        rectangle(x + 2, 118, 7, 7, color);
        for (bar = 0; bar < 16; ++bar) {
            color = 0x100818;
            if (levels[track] > bar * 8) color = fire[bar];
            rectangle(x + 1, 204 - bar * 4, 9, 3, color);
            if (levels[track] > bar * 8) {
                rectangle(x + 2, 204 - bar * 4, 7, 1, fire[bar] | 0x303030);
            }
            if (peaks[track] != 0 && bar == (peaks[track] - 1u) / 8) {
                rectangle(x + 1, 204 - bar * 4, 9, 1, 0xa0e8ff);
            }
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
    background(frames);
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
