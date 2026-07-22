#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define MKDIR(d) _mkdir(d)
#else
#include <sys/stat.h>
#include <dirent.h>
#define MKDIR(d) mkdir(d, 0755)
#endif

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "audio.h"
#include "util.h"
#include "emi.h"
#include "psf.h"
#include "psx_machine.h"

/* ── audio source abstraction ─────────────────────────────────────── */

static char g_root_dir[512] = ".";

static int ends_with(const char *s, const char *suffix)
{
    size_t sl = strlen(s), xl = strlen(suffix);
    if (xl > sl) return 0;
    for (size_t i = 0; i < xl; i++) {
        char a = s[sl - xl + i], b = suffix[i];
        if (a >= 'A' && a <= 'Z') a += 32;
        if (b >= 'A' && b <= 'Z') b += 32;
        if (a != b) return 0;
    }
    return 1;
}

typedef struct {
    uint8_t *owned_vh, *owned_vb, *owned_sep;
    const uint8_t *vh, *vb, *sep;
    uint32_t vh_sz, vb_sz, sep_sz;
} AudioSource;

static void source_free(AudioSource *s)
{
    free(s->owned_vh);
    free(s->owned_vb);
    free(s->owned_sep);
    memset(s, 0, sizeof(*s));
}

static int source_from_emi(AudioSource *s, const char *path)
{
    size_t len;
    EmiFile emi;

    memset(s, 0, sizeof(*s));
    s->owned_vh = read_file(path, &len);
    if (!s->owned_vh) return -1;
    if (emi_parse(s->owned_vh, len, &emi) != 0) {
        source_free(s);
        return -1;
    }
    s->vh  = emi_find_type(&emi, EMI_TYPE_VH,  &s->vh_sz);
    s->vb  = emi_find_type(&emi, EMI_TYPE_VB,  &s->vb_sz);
    s->sep = emi_find_type(&emi, EMI_TYPE_SEQ, &s->sep_sz);
    if (!s->vh || !s->vb) { source_free(s); return -1; }
    return 0;
}

static int source_from_raw(AudioSource *s, const char *vh_p, const char *vb_p, const char *sep_p)
{
    size_t vl, bl, sl;

    memset(s, 0, sizeof(*s));
    uint8_t *vh = read_file(vh_p, &vl);
    uint8_t *vb = read_file(vb_p, &bl);
    uint8_t *sep = sep_p ? read_file(sep_p, &sl) : NULL;
    if (!vh || !vb) { free(vh); free(vb); free(sep); return -1; }
    s->owned_vh = vh;
    s->owned_vb = vb;
    s->owned_sep = sep;
    s->vh = vh; s->vh_sz = (uint32_t)vl;
    s->vb = vb; s->vb_sz = (uint32_t)bl;
    s->sep = sep; s->sep_sz = sep ? (uint32_t)sl : 0;
    return 0;
}

static int source_from_dir(AudioSource *s, const char *dir)
{
    char vh_p[512], vb_p[512], sep_p[512];
    int found_vh = 0, found_vb = 0, found_sep = 0;
#ifndef _WIN32
    DIR *d = opendir(dir);
    struct dirent *ent;
    if (!d) return -1;
    while ((ent = readdir(d)) != NULL) {
        char full[600];
        uint8_t hdr[4];
        FILE *f;
        if (!ends_with(ent->d_name, ".bin")) continue;
        snprintf(full, sizeof(full), "%s/%s", dir, ent->d_name);
        f = fopen(full, "rb");
        if (!f) continue;
        if (fread(hdr, 1, 4, f) != 4) { fclose(f); continue; }
        fclose(f);
        if (memcmp(hdr, "\x70\x42\x41\x56", 4) == 0) { snprintf(vh_p, sizeof(vh_p), "%s", full); found_vh = 1; }
        else if (memcmp(hdr, "\x70\x51\x45\x53", 4) == 0) { snprintf(sep_p, sizeof(sep_p), "%s", full); found_sep = 1; }
        else { snprintf(vb_p, sizeof(vb_p), "%s", full); found_vb = 1; }
    }
    closedir(d);
#endif
    if (!found_vh || !found_vb) return -1;
    return source_from_raw(s, vh_p, vb_p, found_sep ? sep_p : NULL);
}

static int source_auto(AudioSource *s, const char *path)
{
    FILE *f = fopen(path, "rb");
    if (f) {
        uint8_t hdr[16];
        size_t n = fread(hdr, 1, 16, f);
        fclose(f);
        if (n >= 16 && emi_check_magic(hdr, n))
            return source_from_emi(s, path);
    }
    if (source_from_dir(s, path) == 0)
        return 0;
    return source_from_emi(s, path);
}

/* ── playback ─────────────────────────────────────────────────────── */

static int16_t *g_pcm;
static int64_t g_pos, g_total;
static int g_channels;

static void data_callback(ma_device *dev, void *out, const void *in, ma_uint32 frames)
{
    int16_t *dst = (int16_t *)out;
    int needed = (int)(frames * (ma_uint32)g_channels);
    int avail = (int)(g_total - g_pos);
    int n = needed < avail ? needed : avail;
    (void)dev; (void)in;
    if (n > 0) { memcpy(dst, g_pcm + g_pos, (size_t)n * sizeof(int16_t)); g_pos += n; }
    if (n < needed) memset(dst + n, 0, (size_t)(needed - n) * sizeof(int16_t));
}

static int play_buffer(int16_t *pcm, int64_t frames, int channels, int rate, float gain)
{
    ma_device_config cfg;
    ma_device dev;

    if (gain != 1.0f) {
        int64_t total = frames * channels;
        for (int64_t i = 0; i < total; i++) {
            float v = (float)pcm[i] * gain;
            pcm[i] = (int16_t)(v > 32767.0f ? 32767 : v < -32768.0f ? -32768 : v);
        }
    }

    g_pcm = pcm; g_pos = 0; g_total = frames * channels; g_channels = channels;
    cfg = ma_device_config_init(ma_device_type_playback);
    cfg.playback.format = ma_format_s16;
    cfg.playback.channels = (ma_uint32)channels;
    cfg.sampleRate = (ma_uint32)rate;
    cfg.dataCallback = data_callback;

    if (ma_device_init(NULL, &cfg, &dev) != MA_SUCCESS) {
        fprintf(stderr, "error: audio device init failed\n");
        return -1;
    }
    ma_device_start(&dev);
    printf("  \xe2\x96\xb6 %.1fs  %dHz  %s  gain=%.1f  [Ctrl+C]\n",
           (double)frames / rate, rate, channels == 2 ? "stereo" : "mono", gain);
    while (g_pos < g_total) ma_sleep(50);
    ma_device_uninit(&dev);
    return 0;
}

static int write_stereo_output(const char *path, const int16_t *pcm,
                               int64_t frames, int rate)
{
    if (ends_with(path, ".ogg"))
        return ogg_write_stereo(path, pcm, frames, rate);
    if (ends_with(path, ".flac"))
        return flac_write_stereo(path, pcm, frames, rate);
    return wav_write_stereo(path, pcm, frames, rate);
}

static int load_game_image(const char *preferred, Psf1Image *image)
{
    static const char *const paths[] = {
        "out/audio/bof3.psflib",
        "../out/audio/bof3.psflib",
        "../../out/audio/bof3.psflib",
        NULL
    };
    int i;

    if (preferred)
        return psf1_load_file(preferred, image) == PSF1_OK ? 0 : -1;
    for (i = 0; paths[i]; i++)
        if (psf1_load_file(paths[i], image) == PSF1_OK)
            return 0;
    return -1;
}

static int play_source(AudioSource *s, int seq_idx, AudioEngine engine,
                       float gain, const char *outpath)
{
    if (s->sep) {
        AudioRenderRequest request;
        AudioRenderResult result;
        Psf1Image game_image;
        AudioStatus status;
        RenderOutput *ro;
        int has_game_image = 0;

        memset(&request, 0, sizeof(request));
        request.engine = engine;
        request.sep_data = s->sep;
        request.sep_len = s->sep_sz;
        request.vh_data = s->vh;
        request.vh_len = s->vh_sz;
        request.vb_data = s->vb;
        request.vb_len = s->vb_sz;
        request.sequence = seq_idx;
        request.output_rate = 44100;
        if (engine == AUDIO_ENGINE_GAME) {
            if (load_game_image(NULL, &game_image) != 0) {
                fprintf(stderr, "error: cannot load out/audio/bof3.psflib\n");
                return -1;
            }
            request.game_image = &game_image;
            has_game_image = 1;
        }
        status = audio_render(&request, &result);
        if (has_game_image)
            psf1_image_free(&game_image);
        if (status != AUDIO_STATUS_OK) {
            fprintf(stderr, "error: %s\n", audio_status_string(status));
            return -1;
        }
        ro = &result.audio;
        int rc;
        if (outpath) {
            if (gain != 1.0f) {
                int64_t total = ro->frames * 2;
                for (int64_t i = 0; i < total; i++) {
                    float v = (float)ro->pcm[i] * gain;
                    ro->pcm[i] = (int16_t)(v > 32767.0f ? 32767 : v < -32768.0f ? -32768 : v);
                }
            }
            if (write_stereo_output(outpath, ro->pcm, ro->frames, ro->rate) != 0) {
                fprintf(stderr, "error: failed to write %s\n", outpath);
                free(ro->pcm);
                return -1;
            }
            printf("  wrote %s (%.1fs, %dHz stereo)\n", outpath,
                   (double)ro->frames / ro->rate, ro->rate);
            rc = 0;
        } else {
            rc = play_buffer(ro->pcm, ro->frames, 2, ro->rate, gain);
        }
        free(ro->pcm);
        return rc;
    }
    VabHeader hdr;
    int16_t *pcm = NULL;
    if (vab_parse_vh(s->vh, s->vh_sz, &hdr) != 0) return -1;
    int n = vab_decode_vag(s->vb, s->vb_sz, &hdr, 0, &pcm);
    if (n <= 0 || !pcm) return -1;
    int rc;
    if (outpath) {
        wav_write_mono(outpath, pcm, n, 44100);
        printf("  wrote %s (%d samples, 44100Hz mono)\n", outpath, n);
        rc = 0;
    } else {
        rc = play_buffer(pcm, n, 1, 44100, gain);
    }
    free(pcm);
    return rc;
}

/* ── args ─────────────────────────────────────────────────────────── */

static const char *arg_str(int argc, char **argv, const char *flag)
{
    for (int i = 1; i < argc - 1; i++)
        if (strcmp(argv[i], flag) == 0) return argv[i + 1];
    return NULL;
}
static int arg_has(int argc, char **argv, const char *flag)
{
    int i;
    for (i = 1; i < argc; i++)
        if (strcmp(argv[i], flag) == 0) return 1;
    return 0;
}
static int arg_int(int argc, char **argv, const char *flag, int def)
{ const char *v = arg_str(argc, argv, flag); return v ? atoi(v) : def; }
static float arg_flt(int argc, char **argv, const char *flag, float def)
{ const char *v = arg_str(argc, argv, flag); return v ? (float)atof(v) : def; }

static float arg_gain(int argc, char **argv)
{
    const char *v = arg_str(argc, argv, "--gain");
    return v ? (float)atof(v) : arg_flt(argc, argv, "-g", 1.0f);
}

static int arg_engine(int argc, char **argv, AudioEngine *engine)
{
    const char *value = arg_str(argc, argv, "--engine");

    if (!value || strcmp(value, "fast") == 0) {
        *engine = AUDIO_ENGINE_FAST;
        return 0;
    }
    if (strcmp(value, "game") == 0) {
        *engine = AUDIO_ENGINE_GAME;
        return 0;
    }
    return -1;
}

/* ── BGM directory scanning ───────────────────────────────────────── */

typedef struct {
    char name[64];
    char path[512];
    int events;
    int tones;
} TrackInfo;

static int find_bgm_dir(char *out, size_t sz)
{
    char cand[600];
    const char *suffixes[] = {
        "out/extracted/BIN/BGM", NULL
    };
    const char *rel_cands[] = {
        "out/extracted/BIN/BGM", "../out/extracted/BIN/BGM",
        "../../out/extracted/BIN/BGM", NULL
    };

    snprintf(cand, sizeof(cand), "%s/%s", g_root_dir, suffixes[0]);
#ifndef _WIN32
    {
        DIR *d = opendir(cand);
        if (d) { closedir(d); snprintf(out, sz, "%s", cand); return 0; }
    }
#endif
    for (int i = 0; rel_cands[i]; i++) {
#ifndef _WIN32
        DIR *d = opendir(rel_cands[i]);
        if (d) { closedir(d); snprintf(out, sz, "%s", rel_cands[i]); return 0; }
#endif
    }
    return -1;
}

static int scan_tracks(const char *dir, TrackInfo *tracks, int max)
{
#ifndef _WIN32
    DIR *d = opendir(dir);
    struct dirent *ent;
    int count = 0;
    if (!d) return 0;
    while ((ent = readdir(d)) != NULL && count < max) {
        char path[512];
        AudioSource s;
        if (!ends_with(ent->d_name, ".emi")) continue;
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
        if (source_from_emi(&s, path) != 0) continue;
        TrackInfo *t = &tracks[count];
        memset(t, 0, sizeof(*t));
        strncpy(t->name, ent->d_name, sizeof(t->name) - 5);
        char *dot = strrchr(t->name, '.');
        if (dot) *dot = '\0';
        strncpy(t->path, path, sizeof(t->path) - 1);
        if (s.vh) {
            VabHeader hdr;
            if (vab_parse_vh(s.vh, s.vh_sz, &hdr) == 0)
                t->tones = (int)hdr.tone_count;
        }
        if (s.sep) {
            SepFile sep;
            if (sep_parse(s.sep, s.sep_sz, &sep) == 0 && sep.sequence_count > 0) {
                t->events = sep.sequences[0].event_count;
                sep_free(&sep);
            }
        }
        source_free(&s);
        count++;
    }
    closedir(d);
    for (int i = 0; i < count - 1; i++)
        for (int j = i + 1; j < count; j++)
            if (strcmp(tracks[i].name, tracks[j].name) > 0) {
                TrackInfo tmp = tracks[i]; tracks[i] = tracks[j]; tracks[j] = tmp;
            }
    return count;
#else
    (void)dir; (void)tracks; (void)max; return 0;
#endif
}

static int find_track_path(const char *name, char *out, size_t sz)
{
    char dir[512], upper[128];
    strncpy(upper, name, sizeof(upper) - 1);
    upper[sizeof(upper) - 1] = '\0';
    for (char *p = upper; *p; p++) if (*p >= 'a' && *p <= 'z') *p -= 32;
    if (ends_with(upper, ".EMI"))
        upper[strlen(upper) - 4] = '\0';

    if (find_bgm_dir(dir, sizeof(dir)) != 0) return -1;

    const char *pats[] = { "%s/%s.EMI", "%s/BGM%s.EMI", "%s/%s", "%s/BGM%s", NULL };
    for (int i = 0; pats[i]; i++) {
        snprintf(out, sz, pats[i], dir, upper);
        FILE *f = fopen(out, "rb");
        if (f) { fclose(f); return 0; }
    }
    return -1;
}

/* ── commands ─────────────────────────────────────────────────────── */

static int cmd_list(int argc, char **argv)
{
    char dir[512];
    const char *filter = argc > 2 ? argv[2] : NULL;
    TrackInfo *tracks;
    int max = 256;

    if (find_bgm_dir(dir, sizeof(dir)) != 0) {
        fprintf(stderr, "error: out/extracted/BIN/BGM not found\n");
        return 1;
    }
    tracks = calloc(max, sizeof(TrackInfo));
    if (!tracks) return 1;
    int count = scan_tracks(dir, tracks, max);

    printf("\n  BGM Tracks (%d)\n", count);
    printf("  %.*s\n", 62, "──────────────────────────────────────────────────────────────");
    printf("  %4s  %-20s %7s %6s\n", "#", "Name", "Events", "Tones");
    printf("  %4s  %-20s %7s %6s\n", "───", "────────────────────", "──────", "─────");

    int shown = 0;
    for (int i = 0; i < count; i++) {
        if (filter) {
            char un[64], uf[64];
            strncpy(un, tracks[i].name, 63); un[63] = '\0';
            strncpy(uf, filter, 63); uf[63] = '\0';
            for (char *p = un; *p; p++) if (*p >= 'a' && *p <= 'z') *p -= 32;
            for (char *p = uf; *p; p++) if (*p >= 'a' && *p <= 'z') *p -= 32;
            if (!strstr(un, uf)) continue;
        }
        printf("  %4d  %-20s %6d  %5d\n", shown, tracks[i].name, tracks[i].events, tracks[i].tones);
        shown++;
    }
    printf("  %.*s\n", 62, "──────────────────────────────────────────────────────────────");
    printf("  bin/psx-audio play <name>    e.g. bin/psx-audio play BGM000\n\n");
    free(tracks);
    return 0;
}

static int cmd_play_bgm(int argc, char **argv)
{
    AudioSource s;
    AudioEngine engine;
    int seq = arg_int(argc, argv, "-s", 0);
    float gain = arg_gain(argc, argv);
    const char *outpath = arg_str(argc, argv, "-o");
    char path[512];
    const char *target = argv[2];
    int rc;

    if (arg_engine(argc, argv, &engine) != 0) {
        fprintf(stderr, "error: --engine must be fast or game\n");
        return 1;
    }

    if (ends_with(target, ".emi") || ends_with(target, ".EMI")) {
        printf("  %s\n", target);
        if (source_from_emi(&s, target) != 0) {
            fprintf(stderr, "error: no audio in %s\n", target); return 1;
        }
    } else if (argc >= 5 && ends_with(target, ".bin")) {
        if (source_from_raw(&s, argv[2], argv[3], argv[4]) != 0) {
            fprintf(stderr, "error: failed to read files\n"); return 1;
        }
        printf("  %s + %s + %s\n", argv[2], argv[3], argv[4]);
    } else {
        if (find_track_path(target, path, sizeof(path)) != 0) {
            fprintf(stderr, "error: track '%s' not found (try: bin/psx-audio list)\n", target);
            return 1;
        }
        printf("  %s\n", path);
        if (source_auto(&s, path) != 0) {
            fprintf(stderr, "error: no audio in %s\n", path); return 1;
        }
    }

    rc = play_source(&s, seq, engine, gain, outpath);
    source_free(&s);
    return rc != 0 ? 1 : 0;
}

static int cmd_play_xa(int argc, char **argv)
{
    uint8_t *data; size_t len;
    int16_t *pcm = NULL; int rate, nch;
    int channel = arg_int(argc, argv, "-c", 0);
    float gain = arg_gain(argc, argv);
    const char *outpath = arg_str(argc, argv, "-o");

    data = read_file(argv[2], &len);
    if (!data) { fprintf(stderr, "error: read failed\n"); return 1; }
    int64_t frames = xa_decode_channel(data, len, channel, &pcm, &rate, &nch);
    free(data);
    if (frames <= 0) { fprintf(stderr, "error: decode failed (ch %d)\n", channel); return 1; }
    if (outpath) {
        if (nch == 1) wav_write_mono(outpath, pcm, frames, rate);
        else wav_write_stereo(outpath, pcm, frames, rate);
        printf("  wrote %s (%.1fs, %dHz, %s)\n", outpath, (double)frames / rate, rate,
               nch == 2 ? "stereo" : "mono");
    } else {
        printf("  %s — ch %d\n", argv[2], channel);
        play_buffer(pcm, frames, nch, rate, gain);
    }
    free(pcm);
    return 0;
}

static int cmd_play_vag(int argc, char **argv)
{
    AudioSource s;
    VabHeader hdr;
    int vag = arg_int(argc, argv, "-v", -1);
    float gain = arg_gain(argc, argv);

    if (source_from_raw(&s, argv[2], argv[3], NULL) != 0) {
        fprintf(stderr, "error: read failed\n"); return 1;
    }
    if (vab_parse_vh(s.vh, s.vh_sz, &hdr) != 0) {
        fprintf(stderr, "error: bad VH\n"); source_free(&s); return 1;
    }

    if (vag >= 0) {
        int16_t *pcm = NULL;
        const char *outpath = arg_str(argc, argv, "-o");
        int n = vab_decode_vag(s.vb, s.vb_sz, &hdr, vag, &pcm);
        if (n > 0 && pcm) {
            if (outpath) {
                wav_write_mono(outpath, pcm, n, 44100);
                printf("  wrote %s (%d samples)\n", outpath, n);
            } else {
                printf("  VAG %d (%d samples)\n", vag, n);
                play_buffer(pcm, n, 1, 44100, gain);
            }
            free(pcm);
        }
    } else {
        for (int i = 0; i < (int)hdr.tone_count; i++) {
            int16_t *pcm = NULL;
            int n = vab_decode_vag(s.vb, s.vb_sz, &hdr, i, &pcm);
            if (n > 0 && pcm) {
                printf("  VAG %d/%d\n", i, (int)hdr.tone_count);
                play_buffer(pcm, n, 1, 44100, gain);
                free(pcm);
            }
        }
    }
    source_free(&s);
    return 0;
}

static int cmd_play(int argc, char **argv)
{
    if (argc < 3) { fprintf(stderr, "usage: play <track|file> [-s N] [-g GAIN]\n"); return 1; }
    if (ends_with(argv[2], ".str") || ends_with(argv[2], ".STR"))
        return cmd_play_xa(argc, argv);
    return cmd_play_bgm(argc, argv);
}

static int cmd_render(int argc, char **argv)
{
    const char *outpath = arg_str(argc, argv, "-o");
    int seq = arg_int(argc, argv, "-s", 0);
    float gain = arg_gain(argc, argv);
    AudioSource s;
    AudioEngine engine;
    AudioRenderRequest request;
    AudioRenderResult result;
    Psf1Image game_image;
    AudioStatus status;
    RenderOutput *ro;
    char path[512];
    int has_game_image = 0;

    if (!outpath) { fprintf(stderr, "error: -o required\n"); return 1; }
    if (arg_engine(argc, argv, &engine) != 0) {
        fprintf(stderr, "error: --engine must be fast or game\n");
        return 1;
    }

    if (argc >= 5 && ends_with(argv[2], ".bin") &&
        source_from_raw(&s, argv[2], argv[3], argv[4]) == 0) {
        /* raw files */
    } else if (source_auto(&s, argv[2]) == 0) {
        /* EMI or directory */
    } else if (find_track_path(argv[2], path, sizeof(path)) == 0 &&
               source_auto(&s, path) == 0) {
        /* resolved track name */
    } else {
        fprintf(stderr, "error: cannot load audio from %s\n", argv[2]); return 1;
    }

    if (!s.sep) { fprintf(stderr, "error: no sequence data\n"); source_free(&s); return 1; }
    memset(&request, 0, sizeof(request));
    request.engine = engine;
    request.sep_data = s.sep;
    request.sep_len = s.sep_sz;
    request.vh_data = s.vh;
    request.vh_len = s.vh_sz;
    request.vb_data = s.vb;
    request.vb_len = s.vb_sz;
    request.sequence = seq;
    request.output_rate = 44100;
    if (engine == AUDIO_ENGINE_GAME) {
        if (load_game_image(arg_str(argc, argv, "--psflib"), &game_image) != 0) {
            fprintf(stderr, "error: cannot load game PSFLib\n");
            source_free(&s);
            return 1;
        }
        request.game_image = &game_image;
        has_game_image = 1;
    }
    status = audio_render(&request, &result);
    if (has_game_image)
        psf1_image_free(&game_image);
    if (status != AUDIO_STATUS_OK) {
        fprintf(stderr, "error: %s\n", audio_status_string(status));
        source_free(&s);
        return 1;
    }
    ro = &result.audio;
    if (gain != 1.0f) {
        int64_t count = ro->frames * 2;
        for (int64_t i = 0; i < count; i++) {
            float value = (float)ro->pcm[i] * gain;
            ro->pcm[i] = (int16_t)(value > 32767.0f ? 32767 : value < -32768.0f ? -32768 : value);
        }
    }
    if (write_stereo_output(outpath, ro->pcm, ro->frames, ro->rate) != 0) {
        fprintf(stderr, "error: failed to write %s (compressed output requires its codec library)\n", outpath);
        free(ro->pcm);
        source_free(&s);
        return 1;
    }
    printf("  wrote %s (%.1fs, %dHz stereo)\n", outpath,
           (double)ro->frames / ro->rate, ro->rate);
    free(ro->pcm);
    source_free(&s);
    return 0;
}

static int cmd_xa_inspect(int argc, char **argv)
{
    uint8_t *data; size_t len;
    XaStreamInfo streams[32];
    data = read_file(argv[2], &len);
    if (!data) { fprintf(stderr, "error: read failed\n"); return 1; }
    int count = xa_inspect(data, len, streams, 32);
    printf("  %s (%zu bytes)\n", argv[2], len);
    for (int i = 0; i < count; i++)
        printf("    ch %d: %dHz %s  %.1fs\n", i, streams[i].rate,
               streams[i].channels == 2 ? "stereo" : "mono  ",
               (double)streams[i].frame_count / streams[i].rate);
    free(data);
    return 0;
}

static int cmd_xa_decode(int argc, char **argv)
{
    const char *outpath = arg_str(argc, argv, "-o");
    int channel = arg_int(argc, argv, "-c", 0);
    uint8_t *data; size_t len;
    int16_t *pcm = NULL; int rate, nch;

    if (!outpath) { fprintf(stderr, "error: -o required\n"); return 1; }
    data = read_file(argv[2], &len);
    if (!data) { fprintf(stderr, "error: read failed\n"); return 1; }
    int64_t frames = xa_decode_channel(data, len, channel, &pcm, &rate, &nch);
    free(data);
    if (frames <= 0) { fprintf(stderr, "error: decode failed\n"); return 1; }
    if (nch == 1) wav_write_mono(outpath, pcm, frames, rate);
    else wav_write_stereo(outpath, pcm, frames, rate);
    printf("  wrote %s (%.1fs, %dHz, %s)\n", outpath, (double)frames / rate, rate,
           nch == 2 ? "stereo" : "mono");
    free(pcm);
    return 0;
}

static int cmd_vab_inspect(int argc, char **argv)
{
    uint8_t *data; size_t len;
    VabHeader hdr;
    data = read_file(argv[2], &len);
    if (!data) { fprintf(stderr, "error: read failed\n"); return 1; }
    if (vab_parse_vh(data, len, &hdr) != 0) { fprintf(stderr, "error: bad VH\n"); free(data); return 1; }
    printf("  %s: programs=%u tones=%u vags=%u file=%u bytes\n", argv[2],
           hdr.program_count, hdr.tone_count, hdr.vag_count, hdr.file_size);
    for (int i = 0; i < (int)hdr.tone_count; i++) {
        VabTone *t = &hdr.tones[i];
        printf("    [%2d] prog=%d block=%d/%d note=%d-%d center=%d shift=%d bend=%d/%d "
               "vib=%d/%d por=%d/%d mode=%02X vag=%u+%u adsr=%04X/%04X\n",
               i, t->prog, t->storage_block, t->tone_slot,
               t->min_note, t->max_note, t->center_note,
               t->shift, t->pitch_bend_min, t->pitch_bend_max,
               t->vibrato_width, t->vibrato_time,
               t->portamento_width, t->portamento_time, t->mode,
               t->vag_offset, t->vag_size, t->adsr1, t->adsr2);
    }
    free(data);
    return 0;
}

static int cmd_bgm_audit(int argc, char **argv)
{
    AudioSource source;
    AudioAuditReport report;
    int program, note;

    if (argc < 3) {
        fprintf(stderr, "usage: bgm-audit <track|EMI|directory>\n");
        return 1;
    }
    memset(&source, 0, sizeof(source));
    if (source_auto(&source, argv[2]) != 0 || !source.sep) {
        fprintf(stderr, "%s: error=missing-or-invalid-EMI/VH/VB/SEP\n", argv[2]);
        source_free(&source);
        return 1;
    }
    if (audio_audit_bgm(source.vh, source.vh_sz, source.vb, source.vb_sz,
                        source.sep, source.sep_sz, &report) != 0) {
        fprintf(stderr, "%s: error=invalid-VH/VB/SEP\n", argv[2]);
        source_free(&source);
        return 1;
    }
    printf("%s: vh=%u+%u/%u programs=%u tones=%u vags=%u seq=%d "
           "remap=%d missing-note-events=%d layered-note-events=%d "
           "bad-vag=%d bad-prefix=%d missing-end=%d reverb-tones=%d "
           "modulation-tones=%d bend-lsb-events=%d ignored-controls=%d "
           "loop-controls=%d\n",
           argv[2], report.vh_size, report.vb_size,
           report.declared_file_size, report.program_count,
           report.tone_count, report.vag_count, report.sequence_count,
           report.remapped_tones, report.missing_note_events,
           report.layered_note_events, report.bad_vag_ranges,
           report.bad_sample_prefixes, report.samples_without_end,
           report.reverb_tones, report.modulation_tones,
           report.bend_lsb_events, report.ignored_control_events,
           report.loop_control_events);
    if (arg_has(argc, argv, "--details") && report.missing_note_events) {
        printf("  missing:");
        for (program = 0; program < 128; program++)
            for (note = 0; note < 128; note++)
                if (report.missing_notes[program][note])
                    printf(" p%d/n%d:%d", program, note,
                           report.missing_notes[program][note]);
        printf("\n");
    }

    source_free(&source);
    return report.bad_vag_ranges || report.bad_sample_prefixes ||
                   report.samples_without_end ||
                   report.declared_file_size != report.vh_size + report.vb_size
               ? 1
               : 0;
}

static int cmd_vab_extract(int argc, char **argv)
{
    const char *outdir = arg_str(argc, argv, "-o");
    AudioSource s;
    VabHeader hdr;
    if (!outdir) { fprintf(stderr, "error: -o required\n"); return 1; }
    if (source_from_raw(&s, argv[2], argv[3], NULL) != 0) {
        fprintf(stderr, "error: read failed\n"); return 1;
    }
    if (vab_parse_vh(s.vh, s.vh_sz, &hdr) != 0) {
        fprintf(stderr, "error: bad VH\n"); source_free(&s); return 1;
    }
    MKDIR(outdir);
    int extracted = 0;
    for (int i = 0; i < (int)hdr.tone_count; i++) {
        int16_t *pcm = NULL;
        int n = vab_decode_vag(s.vb, s.vb_sz, &hdr, i, &pcm);
        if (n > 0 && pcm) {
            char path[512];
            snprintf(path, sizeof(path), "%s/vag_%03d.wav", outdir, i);
            wav_write_mono(path, pcm, n, 44100);
            extracted++;
            free(pcm);
        }
    }
    printf("  extracted %d/%u tones to %s/\n", extracted, hdr.tone_count, outdir);
    source_free(&s);
    return 0;
}

static int cmd_sep_inspect(int argc, char **argv)
{
    uint8_t *data; size_t len;
    SepFile sep;
    data = read_file(argv[2], &len);
    if (!data) { fprintf(stderr, "error: read failed\n"); return 1; }
    if (sep_parse(data, len, &sep) != 0) { fprintf(stderr, "error: bad SEP\n"); free(data); return 1; }
    printf("  %s: %d sequence(s)\n", argv[2], sep.sequence_count);
    for (int i = 0; i < sep.sequence_count; i++) {
        printf("    [%d] res=%d events=%d\n", i, sep.sequences[i].resolution, sep.sequences[i].event_count);
        if (arg_has(argc, argv, "--programs")) {
            int programs[16] = { 0 };
            int note_count[128] = { 0 };
            int note_histogram[128][128] = { { 0 } };
            int min_note[128];
            int max_note[128];
            int event_index;
            int program;

            for (program = 0; program < 128; program++) {
                min_note[program] = 128;
                max_note[program] = -1;
            }
            for (event_index = 0;
                 event_index < sep.sequences[i].event_count;
                 event_index++) {
                SepEvent *event = &sep.sequences[i].events[event_index];
                int channel = event->type & 0x0f;
                if ((event->type & 0xf0) == 0xc0) {
                    programs[channel] = event->data1;
                } else if ((event->type & 0xf0) == 0x90 && event->data2 != 0) {
                    program = programs[channel];
                    note_count[program]++;
                    note_histogram[program][event->data1]++;
                    if (event->data1 < min_note[program])
                        min_note[program] = event->data1;
                    if (event->data1 > max_note[program])
                        max_note[program] = event->data1;
                }
            }
            for (program = 0; program < 128; program++)
                if (note_count[program] != 0) {
                    printf("      program=%d notes=%d range=%d-%d\n",
                           program, note_count[program], min_note[program],
                           max_note[program]);
                    if (arg_has(argc, argv, "--notes")) {
                        int note;
                        printf("        note-counts:");
                        for (note = 0; note < 128; note++)
                            if (note_histogram[program][note] != 0)
                                printf(" %d:%d", note,
                                       note_histogram[program][note]);
                        printf("\n");
                    }
                }
        }
        if (arg_has(argc, argv, "--bends")) {
            int bend_count[16] = { 0 };
            int min_data1[16], max_data1[16];
            int min_data2[16], max_data2[16];
            int event_index;
            int channel;

            for (channel = 0; channel < 16; channel++) {
                min_data1[channel] = min_data2[channel] = 128;
                max_data1[channel] = max_data2[channel] = -1;
            }
            for (event_index = 0;
                 event_index < sep.sequences[i].event_count;
                 event_index++) {
                SepEvent *event = &sep.sequences[i].events[event_index];
                if ((event->type & 0xf0) != 0xe0)
                    continue;
                channel = event->type & 0x0f;
                bend_count[channel]++;
                if (event->data1 < min_data1[channel])
                    min_data1[channel] = event->data1;
                if (event->data1 > max_data1[channel])
                    max_data1[channel] = event->data1;
                if (event->data2 < min_data2[channel])
                    min_data2[channel] = event->data2;
                if (event->data2 > max_data2[channel])
                    max_data2[channel] = event->data2;
            }
            for (channel = 0; channel < 16; channel++)
                if (bend_count[channel] != 0)
                    printf("      bend ch=%d events=%d data1=%d-%d data2=%d-%d\n",
                           channel, bend_count[channel], min_data1[channel],
                           max_data1[channel], min_data2[channel],
                           max_data2[channel]);
            if (arg_has(argc, argv, "--events")) {
                uint32_t tick = 0;
                for (event_index = 0;
                     event_index < sep.sequences[i].event_count;
                     event_index++) {
                    SepEvent *event = &sep.sequences[i].events[event_index];
                    tick += event->delta;
                    if ((event->type & 0xf0) == 0xe0)
                        printf("        bend-event tick=%u ch=%d data1=%u data2=%u\n",
                               tick, event->type & 0x0f, event->data1,
                               event->data2);
                }
            }
        }
        if (arg_has(argc, argv, "--controls")) {
            int controls[128][128] = { { 0 } };
            int event_index;
            int control;
            int value;

            for (event_index = 0;
                 event_index < sep.sequences[i].event_count;
                 event_index++) {
                SepEvent *event = &sep.sequences[i].events[event_index];
                if ((event->type & 0xf0) == 0xb0)
                    controls[event->data1][event->data2]++;
            }
            for (control = 0; control < 128; control++) {
                int count = 0;
                for (value = 0; value < 128; value++)
                    count += controls[control][value];
                if (count == 0)
                    continue;
                printf("      control=%d events=%d values:", control, count);
                for (value = 0; value < 128; value++)
                    if (controls[control][value] != 0)
                        printf(" %d:%d", value, controls[control][value]);
                printf("\n");
            }
        }
    }
    sep_free(&sep); free(data);
    return 0;
}

static int cmd_sep2mid(int argc, char **argv)
{
    const char *outpath = arg_str(argc, argv, "-o");
    int seq = arg_int(argc, argv, "-s", 0);
    uint8_t *data; size_t len;
    SepFile sep;
    if (!outpath) { fprintf(stderr, "error: -o required\n"); return 1; }
    data = read_file(argv[2], &len);
    if (!data) { fprintf(stderr, "error: read failed\n"); return 1; }
    if (sep_parse(data, len, &sep) != 0) { fprintf(stderr, "error: bad SEP\n"); free(data); return 1; }
    if (sep_to_midi(&sep, seq, outpath) != 0) fprintf(stderr, "error: export failed\n");
    else printf("  wrote %s (seq %d)\n", outpath, seq);
    sep_free(&sep); free(data);
    return 0;
}

static int cmd_emi_inspect(int argc, char **argv)
{
    uint8_t *data; size_t len;
    EmiFile emi;
    data = read_file(argv[2], &len);
    if (!data) { fprintf(stderr, "error: read failed\n"); return 1; }
    if (emi_parse(data, len, &emi) != 0) { fprintf(stderr, "error: not an EMI file\n"); free(data); return 1; }
    printf("  %s: %d entries\n", argv[2], emi.count);
    for (int i = 0; i < emi.count; i++)
        printf("    [%d] type=%2d %-12s size=%-8u offset=0x%X\n",
               i, emi.entries[i].type, emi_type_name(emi.entries[i].type),
               emi.entries[i].size, emi.entries[i].offset);
    free(data);
    return 0;
}

static int cmd_psf_inspect(int argc, char **argv)
{
    Psf1Image image;
    Psf1Status status;

    if (argc < 3) {
        fprintf(stderr, "usage: psf-inspect <file.psf>\n");
        return 1;
    }
    status = psf1_load_file(argv[2], &image);
    if (status != PSF1_OK) {
        fprintf(stderr, "error: %s\n", psf1_status_string(status));
        return 1;
    }
    printf("  %s\n", argv[2]);
    printf("    PC:      0x%08X\n", image.initial_pc);
    printf("    SP:      0x%08X\n", image.initial_sp);
    printf("    RAM:     0x%05X-0x%05X\n", image.loaded_min,
           image.loaded_max);
    printf("    refresh: %dHz\n", image.refresh_rate);
    psf1_image_free(&image);
    return 0;
}

static int cmd_psf_pack(int argc, char **argv)
{
    const char *outpath = arg_str(argc, argv, "-o");
    uint8_t *exe;
    size_t exe_size;
    Psf1Status status;

    if (argc < 3 || !outpath) {
        fprintf(stderr, "usage: psf-pack <PS-X EXE> -o <file.psflib>\n");
        return 1;
    }
    exe = read_file(argv[2], &exe_size);
    if (!exe) {
        fprintf(stderr, "error: cannot read %s\n", argv[2]);
        return 1;
    }
    status = psf1_write_file(outpath, exe, exe_size, NULL);
    free(exe);
    if (status != PSF1_OK) {
        fprintf(stderr, "error: %s\n", psf1_status_string(status));
        return 1;
    }
    printf("  wrote %s\n", outpath);
    return 0;
}

static int cmd_psf_run(int argc, char **argv)
{
    int instructions = arg_int(argc, argv, "-n", 100000);
    const char *call_value = arg_str(argc, argv, "--call");
    Psf1Image image;
    Psf1Status image_status;
    PsxSpu *spu;
    PsxMachine *machine;
    PsxMachineStatus machine_status;

    if (argc < 3 || instructions < 0) {
        fprintf(stderr, "usage: psf-run <file.psf> [-n INSTRUCTIONS]\n");
        return 1;
    }
    image_status = psf1_load_file(argv[2], &image);
    if (image_status != PSF1_OK) {
        fprintf(stderr, "error: %s\n", psf1_status_string(image_status));
        return 1;
    }
    spu = psx_spu_create();
    machine = psx_machine_create(&image, spu);
    psf1_image_free(&image);
    if (!spu || !machine) {
        psx_machine_destroy(machine);
        psx_spu_destroy(spu);
        fprintf(stderr, "error: cannot allocate PSX machine\n");
        return 1;
    }
    if (call_value) {
        uint32_t arguments[4] = { 0, 0, 0, 0 };
        uint32_t address = (uint32_t)strtoul(call_value, NULL, 0);
        machine_status = psx_machine_call(machine, address, arguments,
                                          (uint64_t)instructions);
    } else {
        machine_status = psx_machine_run(machine, (uint64_t)instructions);
    }
    printf("  cycles:     %llu\n",
           (unsigned long long)psx_machine_cycles(machine));
    printf("  PC:         0x%08X\n", psx_machine_pc(machine));
    printf("  SPU writes: %zu\n", psx_spu_write_count(spu));
    if (machine_status != PSX_MACHINE_OK) {
        const PsxMachineFault *fault = psx_machine_fault(machine);
        fprintf(stderr, "error: %s at PC=0x%08X instruction=0x%08X address=0x%08X\n",
                psx_machine_status_string(machine_status), fault->pc,
                fault->instruction, fault->address);
    }
    psx_machine_destroy(machine);
    psx_spu_destroy(spu);
    return machine_status == PSX_MACHINE_OK ? 0 : 1;
}

static int cmd_vab2sf2(int argc, char **argv)
{
    const char *outpath = arg_str(argc, argv, "-o");
    const char *name = arg_str(argc, argv, "--name");
    AudioSource s;

    if (!outpath) { fprintf(stderr, "error: -o required\n"); return 1; }
    if (!name) name = "BOF3";

    if (argc >= 4 && source_from_raw(&s, argv[2], argv[3], NULL) == 0) {
        /* raw VH + VB files */
    } else if (source_auto(&s, argv[2]) == 0) {
        /* EMI or directory */
    } else {
        fprintf(stderr, "error: cannot load VAB from %s\n", argv[2]);
        return 1;
    }

    if (vab_to_sf2(s.vh, s.vh_sz, s.vb, s.vb_sz, outpath, name) != 0) {
        fprintf(stderr, "error: SF2 export failed\n");
        source_free(&s);
        return 1;
    }
    printf("  wrote %s\n", outpath);
    source_free(&s);
    return 0;
}

/* ── help ─────────────────────────────────────────────────────────── */

static void usage(void)
{
    printf(
        "bof3-audio — PSX audio player, decoder, and exporter\n"
        "\n"
        "usage: bof3-audio <command> [args]\n"
        "\n"
        "browse:\n"
        "  list [filter]                         list BGM tracks\n"
        "  emi-inspect <file.EMI>                show EMI contents\n"
        "  psf-inspect <file.psf>                load PSF1/MiniPSF image\n"
        "  psf-run <file.psf> [-n N] [--call A]  run or call into a bounded PSF1 image\n"
        "\n"
        "play:\n"
        "  play <target> [-s N] [-g GAIN]        play (auto-detects format)\n"
        "  play-bgm <track|EMI|vh vb sep>        play BGM music\n"
        "  play-xa <file.STR> [-c CH]            play XA stream\n"
        "  play-vag <vh> <vb> [-v N]             play VAB samples\n"
        "\n"
        "export:\n"
        "  render <target> -o FILE                render BGM to WAV, Ogg Vorbis, or FLAC\n"
        "  psf-pack <PS-X EXE> -o FILE            package a PSF1 executable\n"
        "  xa-decode <str> -o out.wav [-c CH]    decode XA to WAV\n"
        "  vab-extract <vh> <vb> -o DIR          extract VAGs to WAV\n"
        "  vab2sf2 <vh> <vb|EMI> -o out.sf2     export SoundFont 2\n"
        "  sep2mid <sep> -o out.mid [-s N]       export to MIDI\n"
        "\n"
        "inspect:\n"
        "  xa-inspect <str>                      list XA streams\n"
        "  vab-inspect <vh>                      show VAB info\n"
        "  bgm-audit <track|EMI|directory> [--details]\n"
        "                                        audit VAB/SEP consistency and renderer gaps\n"
        "  sep-inspect <sep> [--programs] [--notes] [--bends] [--controls] [--events]\n"
        "                                        show SEP events and histograms\n"
        "\n"
        "options:\n"
        "  --engine fast|game  BGM renderer (default: fast)\n"
        "  -s N       sequence index (default: 0)\n"
        "  -g, --gain GAIN  playback gain (default: 1.0)\n"
        "  -c CH      XA channel (default: 0)\n"
        "  -v N       VAG index\n"
        "  -o PATH    output file\n"
        "\n"
        "run 'bof3-audio --examples' for usage examples\n");
}

static void examples(void)
{
    printf(
        "examples:\n"
        "\n"
        "  # browse all BGM tracks\n"
        "  bin/psx-audio list\n"
        "  bin/psx-audio list BAT          # filter by name\n"
        "\n"
        "  # play BGM by track name (searches out/extracted/BIN/BGM/)\n"
        "  bin/psx-audio play BGM000\n"
        "  bin/psx-audio play BGMBAT06 -g 0.5\n"
        "  bin/psx-audio play BGMBAT02 --gain 0.7\n"
        "  bin/psx-audio play BGMOPN -s 1  # play sequence 1\n"
        "\n"
        "  # play directly from EMI archive (no extraction needed)\n"
        "  bin/psx-audio play out/extracted/BIN/BGM/BGM000.EMI\n"
        "  bin/psx-audio play-bgm out/extracted/BIN/BGM/BGMBAT06.EMI\n"
        "\n"
        "  # play from extracted directory\n"
        "  bin/psx-audio play out/extracted/BIN/BGM/BGM000\n"
        "\n"
        "  # play from raw files\n"
        "  bin/psx-audio play-bgm 0.bin 2.bin 1.bin\n"
        "\n"
        "  # play XA streaming audio\n"
        "  bin/psx-audio play out/extracted/BIN/SCE_XA/VOICE.STR -c 0\n"
        "  bin/psx-audio play-xa out/extracted/BIN/SCE_XA/S_XA00.STR -c 3\n"
        "\n"
        "  # play individual VAB samples\n"
        "  bin/psx-audio play-vag 0.bin 2.bin -v 0\n"
        "\n"
        "  # render to WAV\n"
        "  bin/psx-audio render BGM000 -o bgm000.wav\n"
        "  bin/psx-audio render BGM000 -o bgm000.ogg\n"
        "  bin/psx-audio render BGM000 -o bgm000.flac\n"
        "  bin/psx-audio render out/extracted/BIN/BGM/BGMOPN.EMI -o opening.wav\n"
        "\n"
        "  # export\n"
        "  bin/psx-audio xa-decode VOICE.STR -o voice_ch0.wav -c 0\n"
        "  bin/psx-audio vab-extract 0.bin 2.bin -o samples/\n"
        "  bin/psx-audio vab2sf2 BGM000.EMI -o bgm000.sf2\n"
        "  bin/psx-audio sep2mid 1.bin -o track.mid\n"
        "\n"
        "  # inspect\n"
        "  bin/psx-audio emi-inspect out/extracted/BIN/BGM/BGM000.EMI\n"
        "  bin/psx-audio xa-inspect out/extracted/BIN/SCE_XA/VOICE.STR\n"
        "  bin/psx-audio vab-inspect 0.bin\n"
        "  bin/psx-audio sep-inspect 1.bin\n");
}

/* ── main ─────────────────────────────────────────────────────────── */

int main(int argc, char **argv)
{
    if (argc < 2) { usage(); return 0; }

    const char *cmd = argv[1];

    if (strcmp(cmd, "list") == 0)         return cmd_list(argc, argv);
    if (strcmp(cmd, "play") == 0)         return cmd_play(argc, argv);
    if (strcmp(cmd, "play-bgm") == 0)     return cmd_play_bgm(argc, argv);
    if (strcmp(cmd, "play-xa") == 0)      return cmd_play_xa(argc, argv);
    if (strcmp(cmd, "play-vag") == 0)     return cmd_play_vag(argc, argv);
    if (strcmp(cmd, "render") == 0)       return cmd_render(argc, argv);
    if (strcmp(cmd, "xa-decode") == 0)    return cmd_xa_decode(argc, argv);
    if (strcmp(cmd, "xa-inspect") == 0)   return cmd_xa_inspect(argc, argv);
    if (strcmp(cmd, "vab-extract") == 0)  return cmd_vab_extract(argc, argv);
    if (strcmp(cmd, "vab-inspect") == 0)  return cmd_vab_inspect(argc, argv);
    if (strcmp(cmd, "bgm-audit") == 0)    return cmd_bgm_audit(argc, argv);
    if (strcmp(cmd, "sep-inspect") == 0)  return cmd_sep_inspect(argc, argv);
    if (strcmp(cmd, "sep2mid") == 0)      return cmd_sep2mid(argc, argv);
    if (strcmp(cmd, "emi-inspect") == 0)  return cmd_emi_inspect(argc, argv);
    if (strcmp(cmd, "psf-inspect") == 0)  return cmd_psf_inspect(argc, argv);
    if (strcmp(cmd, "psf-pack") == 0)     return cmd_psf_pack(argc, argv);
    if (strcmp(cmd, "psf-run") == 0)      return cmd_psf_run(argc, argv);
    if (strcmp(cmd, "vab2sf2") == 0)      return cmd_vab2sf2(argc, argv);
    if (strcmp(cmd, "--examples") == 0)   { examples(); return 0; }
    if (strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0 || strcmp(cmd, "help") == 0)
        { usage(); return 0; }

    fprintf(stderr, "unknown command: %s\n\n", cmd);
    usage();
    return 1;
}
