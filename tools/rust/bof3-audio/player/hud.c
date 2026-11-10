#include "config.h"
#include "hud.h"
#include "graphics.h"
#include "sfx.h"
#include "vocal.h"

#if PLAYER_MODE_MUSIC
static u8 levels[AUDIO_CHANNELS];
static u8 peaks[AUDIO_CHANNELS];
static u8 peak_hold[AUDIO_CHANNELS];
static const struct MusicTiming {
    u32 resolution;
    u32 numerator;
    u32 denominator_power;
} timing[PLAYER_SEQUENCE_COUNT] = PLAYER_TIMING;
#endif

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

#if PLAYER_MODE_MUSIC || PLAYER_MODE_SFX
static void decimal(char *destination, u32 value, u32 digits)
{
    while (digits != 0) {
        destination[--digits] = (char)('0' + value % 10);
        value /= 10;
    }
}
#endif

#if PLAYER_MODE_MUSIC
static void musical_position(u32 ticks)
{
    u32 quarter, denominator, beats, remainder;
    const struct MusicTiming *selected;
    char digits[16];
    selected = &timing[player_status.track];
    quarter = selected->resolution * 4;
    denominator = 1u << selected->denominator_power;
    remainder = ticks % quarter * denominator;
    beats = ticks / quarter * denominator + remainder / quarter;
    decimal(digits, beats / selected->numerator + 1, 4);
    digits[4] = ':';
    decimal(digits + 5, beats % selected->numerator + 1, 3);
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
        graphics_panel(x - 1, 116, 13, 95, 0x100610, 0x080308, 0);
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
    char track[3];
    graphics_panel(176, 16, 132, 78, 0x180818, 0x080008, 1);
    graphics_panel(10, 92, 300, 18, 0x180818, 0x080008, 0);
    graphics_logo();
    graphics_text(184, 22, "PLAYER " PLAYER_VERSION);
    graphics_text_scaled(184, 32, player_status.paused ? "PAUSED" : "PLAYING", 6, 6);
    decimal(track, player_status.track + 1, 2);
    track[2] = 0;
    graphics_text_scaled(240, 32, "TRACK", 6, 6);
    graphics_text_scaled(276, 32, track, 6, 6);
    graphics_text(184, 42, "deitydragon");
    timecode(frames);
    musical_position(ticks);
    graphics_text(16, 96, PLAYER_SONG);
    meters(amplitudes);
    graphics_text_scaled(12, 86, "Audio driver: Capcom C 1993", 6, 6);
    graphics_panel(10, 226, 300, 12, 0x281028, 0x100010, 1);
    graphics_text_scaled(18, 229, "L1 CONTROLS  CROSS PAUSE  SQUARE RESTART", 6, 6);
}
#elif PLAYER_MODE_SFX
void hud_sfx(void)
{
    char number[4];
    const struct SfxSample *sample;
    static const char hex[] = "0123456789ABCDEF";
    sample = sfx_sample();
    graphics_panel(176, 16, 132, 78, 0x180818, 0x080008, 1);
    graphics_logo();
    graphics_text(184, 22, "PLAYER " PLAYER_VERSION);
    graphics_text_scaled(184, 32, "SFX BANK VIEW", 6, 6);
    graphics_text(184, 42, "deitydragon");
    timecode(player_status.position_frames);
    graphics_panel(10, 96, 300, 116, 0x281028, 0x100010, 1);
    graphics_text(18, 104, sfx_bank()->name);
    graphics_text(18, 124, "BANK");
    decimal(number, player_status.bank + 1, 3);
    number[3] = 0;
    graphics_text(58, 124, number);
    graphics_text(98, 124, "SLOT");
    decimal(number, sfx_bank()->asset.bank, 3);
    graphics_text(138, 124, number);
    graphics_text(18, 144, "SAMPLE");
    decimal(number, player_status.sample + 1, 3);
    graphics_text(74, 144, number);
    graphics_text(114, 144, "OF");
    decimal(number, sfx_bank()->sample_count, 3);
    graphics_text(138, 144, number);
    if (sample->playback == SFX_GAME_CUE) {
        number[0] = hex[(sample->cue >> 8) & 15];
        number[1] = hex[(sample->cue >> 4) & 15];
        number[2] = hex[sample->cue & 15];
        graphics_text(18, 164, "GAME CUE");
        graphics_text(98, 164, number);
    } else if (sample->playback == SFX_SDK_NOTE) {
        graphics_text(18, 164, "SDK NOTE");
        graphics_text_scaled(18, 180, "CUE UNRESOLVED", 6, 6);
    } else graphics_text(18, 164, sample->playback == SFX_AMBIGUOUS ?
        "MULTIPLE CUE BINDINGS" : "SAMPLE BINDING UNRESOLVED");
    graphics_text(18, 196, player_status.error ? "DRIVER ERROR" :
        (player_status.playing ? "PLAYING SAMPLE" : "IDLE"));
    if (player_status.playing && !player_status.error) {
        decimal(number, player_status.playing_sample, 3);
        graphics_text(138, 196, number);
    }
    graphics_text_scaled(12, 86, "Audio driver: Capcom C 1993", 6, 6);
    graphics_panel(10, 226, 300, 12, 0x281028, 0x100010, 1);
    graphics_text_scaled(18, 229, "L1 CONTROLS  CROSS PLAY  SQUARE HALT", 6, 6);
}
#elif PLAYER_MODE_VOCAL
void hud_vocal(void)
{
    const char *state;
    if (player_status.error) state = "CD INIT FAILED";
    else if (!player_status.playing) state = "IDLE";
    else if (vocal_status.state >= 6) state = "STOPPING";
    else if (vocal_status.state == 5) state = "PLAYING XA";
    else state = "STARTING";

    graphics_panel(176, 16, 132, 78, 0x180818, 0x080008, 1);
    graphics_logo();
    graphics_text(184, 22, "PLAYER " PLAYER_VERSION);
    graphics_text_scaled(184, 32, "VOCAL VIEW", 6, 6);
    graphics_text(184, 42, "deitydragon");
    timecode(player_status.position_frames);
    graphics_panel(10, 96, 300, 116, 0x281028, 0x100010, 1);
    graphics_text(18, 104, PLAYER_SONG);
    graphics_text(18, 124, "CUE " PLAYER_VOCAL_ID);
    graphics_text(18, 144, "JAPANESE VOCALS");
    graphics_text_scaled(18, 164, PLAYER_VOCAL_FORMAT, 6, 6);
    graphics_text(18, 184, state);
    graphics_text_scaled(188, 185, vocal_status.repeat ? "REPEAT ON" : "REPEAT OFF", 6, 6);
    graphics_text_scaled(12, 86, "Audio driver: Capcom C 1993", 6, 6);
    graphics_panel(10, 226, 300, 12, 0x281028, 0x100010, 1);
    graphics_text_scaled(18, 229, "L1 CONTROLS  CROSS PLAY  CIRCLE STOP", 6, 6);
}
#endif

void hud_help(void)
{
    graphics_panel(32, 42, 256, 152, 0x602040, 0x301020, 1);
#if PLAYER_MODE_SFX
    graphics_text(48, 56, "SFX CONTROLS");
    graphics_text(48, 80, "LEFT/RIGHT SWITCH BANK");
    graphics_text(48, 100, "UP/DOWN SELECT SAMPLE");
    graphics_text(48, 120, "CROSS   PLAY SAMPLE");
    graphics_text(48, 140, "SQUARE  HALT");
#elif PLAYER_MODE_VOCAL
    graphics_text(48, 56, "VOCAL CONTROLS");
    graphics_text(48, 80, "CROSS   PLAY/RETRIGGER");
    graphics_text(48, 100, "CIRCLE  STOP");
    graphics_text(48, 120, "SQUARE  TOGGLE REPEAT");
    graphics_text(48, 140, "ONE QUALIFIED CUE");
#else
    graphics_text(48, 56, "MUSIC CONTROLS");
    graphics_text(48, 80, "UP/DOWN PREV/NEXT TRACK");
    graphics_text(48, 104, "CROSS   PLAY/PAUSE");
    graphics_text(48, 128, "SQUARE  RESTART TRACK");
#endif
    graphics_text(48, 174, "L1      CLOSE CONTROLS");
}
