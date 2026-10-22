#include "config.h"
#include "hud.h"
#include "graphics.h"

static u8 levels[AUDIO_CHANNELS];
static u8 peaks[AUDIO_CHANNELS];
static u8 peak_hold[AUDIO_CHANNELS];

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
    graphics_text_scaled(184, 54, "SMPTE 30 NDF", 6, 6);
    graphics_text(184, 64, digits);
}

static void decimal(char *destination, u32 value, u32 digits)
{
    while (digits != 0) {
        destination[--digits] = (char)('0' + value % 10);
        value /= 10;
    }
}

static void musical_position(u32 ticks)
{
    u32 quarter, denominator, beats, remainder;
    char digits[16];
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
    graphics_text_scaled(184, 76, "BAR:BEAT:TICK", 6, 6);
    graphics_text_scaled(184, 84, digits, 6, 6);
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

static void meters(const u8 current[AUDIO_CHANNELS])
{
    static const u32 fire[16] = {
        0x080c90, 0x080ea8, 0x0814c0, 0x081dd8,
        0x0828f0, 0x0838ff, 0x084aff, 0x085eff,
        0x0874ff, 0x088cff, 0x08a4ff, 0x10baff,
        0x20ceff, 0x38deff, 0x58ecff, 0x80f8ff
    };
    u32 track, level, x, bar, color;
    char number[3];
    for (track = 0; track < AUDIO_CHANNELS; ++track) {
        level = meter_level(current[track]);
        if (level >= levels[track]) levels[track] = (u8)level;
        else levels[track] = levels[track] > 4 ? levels[track] - 4 : 0;
        if (levels[track] >= peaks[track]) {
            peaks[track] = levels[track];
            peak_hold[track] = 24;
        } else if (peak_hold[track] != 0) --peak_hold[track];
        else if (peaks[track] != 0) --peaks[track];
        x = 17 + track * 18;
        graphics_shade(x - 1, 116, 13, 95);
        graphics_rectangle(x - 1, 116, 1, 95, 0x100610);
        graphics_rectangle(x + 11, 116, 1, 95, 0x100610);
        color = level != 0 ? fire[level / 8] : 0x100818;
        graphics_rectangle(x + 2, 118, 7, 7, color);
        for (bar = 0; bar < 16; ++bar) {
            color = 0x100818;
            if (levels[track] > bar * 8) color = fire[bar];
            graphics_rectangle(x + 1, 204 - bar * 4, 9, 3, color);
            if (levels[track] > bar * 8) {
                graphics_rectangle(x + 2, 204 - bar * 4, 7, 1, fire[bar] | 0x303030);
            }
            if (peaks[track] != 0 && bar == (peaks[track] - 1u) / 8) {
                graphics_rectangle(x + 1, 204 - bar * 4, 9, 1, 0xa0e8ff);
            }
        }
        number[0] = (char)('0' + (track + 1) / 10);
        number[1] = (char)('0' + (track + 1) % 10);
        number[2] = 0;
        graphics_text(x - 2, 216, number);
    }
}

void hud_draw(u32 frames, u32 ticks, const u8 amplitudes[AUDIO_CHANNELS])
{
    graphics_shade(176, 16, 132, 78);
    graphics_shade(10, 92, 300, 18);
    graphics_logo();
    graphics_text(184, 22, "PLAYER " PLAYER_VERSION);
    graphics_text(184, 42, "deitydragon");
    timecode(frames);
    musical_position(ticks);
    graphics_text(16, 96, PLAYER_SONG);
    meters(amplitudes);
    graphics_text_scaled(82, 230, "Audio driver: Capcom C 1993", 6, 6);
}
