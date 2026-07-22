#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "audio.h"
#include "util.h"

static int16_t *g_pcm;
static int64_t g_pos, g_total;
static int g_channels;

static void data_callback(ma_device *dev, void *out, const void *in, ma_uint32 frames)
{
    int16_t *dst = (int16_t *)out;
    int needed = (int)(frames * (ma_uint32)g_channels);
    int avail = (int)(g_total - g_pos);
    int n = needed < avail ? needed : avail;
    (void)dev;
    (void)in;
    if (n > 0) {
        memcpy(dst, g_pcm + g_pos, (size_t)n * sizeof(int16_t));
        g_pos += n;
    }
    if (n < needed)
        memset(dst + n, 0, (size_t)(needed - n) * sizeof(int16_t));
}

static int play_pcm_buffer(int16_t *pcm, int64_t frames, int channels, int rate, float gain)
{
    ma_device_config config;
    ma_device device;
    int64_t i, total_samples = frames * channels;

    if (gain != 1.0f) {
        for (i = 0; i < total_samples; i++) {
            float v = (float)pcm[i] * gain;
            if (v > 32767.0f) v = 32767.0f;
            if (v < -32768.0f) v = -32768.0f;
            pcm[i] = (int16_t)v;
        }
    }

    g_pcm = pcm;
    g_pos = 0;
    g_total = total_samples;
    g_channels = channels;

    config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_s16;
    config.playback.channels = (ma_uint32)channels;
    config.sampleRate = (ma_uint32)rate;
    config.dataCallback = data_callback;

    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        fprintf(stderr, "error: failed to init audio device\n");
        return -1;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        fprintf(stderr, "error: failed to start audio device\n");
        ma_device_uninit(&device);
        return -1;
    }

    while (g_pos < g_total)
        ma_sleep(100);

    ma_device_uninit(&device);
    return 0;
}

static const char *arg_flag(int argc, char **argv, const char *flag)
{
    int i;
    for (i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], flag) == 0)
            return argv[i + 1];
    }
    return NULL;
}

static int arg_int(int argc, char **argv, const char *flag, int def)
{
    const char *v = arg_flag(argc, argv, flag);
    return v ? atoi(v) : def;
}

static float arg_float(int argc, char **argv, const char *flag, float def)
{
    const char *v = arg_flag(argc, argv, flag);
    return v ? (float)atof(v) : def;
}

static void make_dir(const char *d)
{
#ifdef _WIN32
    _mkdir(d);
#else
    mkdir(d, 0755);
#endif
}

static void usage(void)
{
    fprintf(stderr,
        "usage: bof3-audio <command> [args]\n"
        "\n"
        "  play <vh> <vb> <sep> [-s SEQ] [-g GAIN]     render BGM + play\n"
        "  play-xa <str> [-c CHANNEL] [-g GAIN]        decode XA + play\n"
        "  play-vag <vh> <vb> [-v VAG] [-g GAIN]       play VAG sample(s)\n"
        "  render <vh> <vb> <sep> -o out.wav [-s SEQ]   render BGM to WAV\n"
        "  xa-decode <str> -o out.wav [-c CHANNEL]      decode XA to WAV\n"
        "  xa-inspect <str>                             list XA streams\n"
        "  vab-extract <vh> <vb> -o DIR                 extract VAGs to WAV\n"
        "  vab-inspect <vh>                             show VAB info\n"
        "  sep-inspect <sep>                            show SEP info\n"
        "  sep2mid <sep> -o out.mid [-s SEQ]            export to MIDI\n");
}

static int cmd_play(int argc, char **argv)
{
    uint8_t *vh, *vb, *sep;
    size_t vh_len, vb_len, sep_len;
    int seq_idx;
    float gain;
    RenderOutput ro;

    if (argc < 5) { usage(); return 1; }

    vh = read_file(argv[2], &vh_len);
    vb = read_file(argv[3], &vb_len);
    sep = read_file(argv[4], &sep_len);
    if (!vh || !vb || !sep) {
        fprintf(stderr, "error: failed to read input files\n");
        return 1;
    }

    seq_idx = arg_int(argc, argv, "-s", 0);
    gain = arg_float(argc, argv, "-g", 1.0f);

    if (render_bgm(sep, sep_len, vh, vh_len, vb, vb_len, seq_idx, 44100, &ro) != 0) {
        fprintf(stderr, "error: render failed\n");
        return 1;
    }

    printf("playing %lld frames at %d Hz...\n", (long long)ro.frames, ro.rate);
    play_pcm_buffer(ro.pcm, ro.frames, 2, ro.rate, gain);
    free(ro.pcm);
    free(vh);
    free(vb);
    free(sep);
    return 0;
}

static int cmd_play_xa(int argc, char **argv)
{
    uint8_t *data;
    size_t len;
    int16_t *pcm = NULL;
    int rate, nch, channel;
    float gain;

    if (argc < 3) { usage(); return 1; }

    data = read_file(argv[2], &len);
    if (!data) { fprintf(stderr, "error: failed to read file\n"); return 1; }

    channel = arg_int(argc, argv, "-c", 0);
    gain = arg_float(argc, argv, "-g", 1.0f);

    if (xa_decode_channel(data, len, channel, &pcm, &rate, &nch) != 0) {
        fprintf(stderr, "error: XA decode failed\n");
        free(data);
        return 1;
    }

    {
        int64_t frames = (int64_t)(len / 2336) * 224 / (int64_t)nch;
        if (frames < 1) frames = 1;
        printf("playing XA channel %d (%d Hz, %d ch)...\n", channel, rate, nch);
        play_pcm_buffer(pcm, frames, nch, rate, gain);
    }

    free(pcm);
    free(data);
    return 0;
}

static int cmd_play_vag(int argc, char **argv)
{
    uint8_t *vh, *vb;
    size_t vh_len, vb_len;
    VabHeader hdr;
    int vag_idx, i;
    float gain;

    if (argc < 4) { usage(); return 1; }

    vh = read_file(argv[2], &vh_len);
    vb = read_file(argv[3], &vb_len);
    if (!vh || !vb) { fprintf(stderr, "error: failed to read files\n"); return 1; }

    if (vab_parse_vh(vh, vh_len, &hdr) != 0) {
        fprintf(stderr, "error: failed to parse VH\n");
        return 1;
    }

    vag_idx = arg_int(argc, argv, "-v", -1);
    gain = arg_float(argc, argv, "-g", 1.0f);

    if (vag_idx >= 0) {
        int16_t *pcm = NULL;
        int n = vab_decode_vag(vb, vb_len, &hdr, vag_idx, &pcm);
        if (n > 0 && pcm) {
            printf("playing VAG %d (%d frames)...\n", vag_idx, n);
            play_pcm_buffer(pcm, n, 1, 44100, gain);
            free(pcm);
        }
    } else {
        for (i = 0; i < (int)hdr.ps_count; i++) {
            int16_t *pcm = NULL;
            int n = vab_decode_vag(vb, vb_len, &hdr, i, &pcm);
            if (n > 0 && pcm) {
                printf("playing VAG %d/%d (%d frames)...\n", i, (int)hdr.ps_count, n);
                play_pcm_buffer(pcm, n, 1, 44100, gain);
                free(pcm);
            }
        }
    }

    free(vh);
    free(vb);
    return 0;
}

static int cmd_render(int argc, char **argv)
{
    uint8_t *vh, *vb, *sep;
    size_t vh_len, vb_len, sep_len;
    const char *outpath;
    int seq_idx;
    RenderOutput ro;

    if (argc < 5) { usage(); return 1; }

    outpath = arg_flag(argc, argv, "-o");
    if (!outpath) { fprintf(stderr, "error: -o required\n"); return 1; }

    vh = read_file(argv[2], &vh_len);
    vb = read_file(argv[3], &vb_len);
    sep = read_file(argv[4], &sep_len);
    if (!vh || !vb || !sep) {
        fprintf(stderr, "error: failed to read input files\n");
        return 1;
    }

    seq_idx = arg_int(argc, argv, "-s", 0);

    if (render_bgm(sep, sep_len, vh, vh_len, vb, vb_len, seq_idx, 44100, &ro) != 0) {
        fprintf(stderr, "error: render failed\n");
        return 1;
    }

    if (wav_write_stereo(outpath, ro.pcm, ro.frames, ro.rate) != 0) {
        fprintf(stderr, "error: failed to write WAV\n");
        free(ro.pcm);
        return 1;
    }

    printf("wrote %s (%lld frames, %d Hz)\n", outpath, (long long)ro.frames, ro.rate);
    free(ro.pcm);
    free(vh);
    free(vb);
    free(sep);
    return 0;
}

static int cmd_xa_decode(int argc, char **argv)
{
    uint8_t *data;
    size_t len;
    const char *outpath;
    int16_t *pcm = NULL;
    int rate, nch, channel;

    if (argc < 3) { usage(); return 1; }

    outpath = arg_flag(argc, argv, "-o");
    if (!outpath) { fprintf(stderr, "error: -o required\n"); return 1; }

    data = read_file(argv[2], &len);
    if (!data) { fprintf(stderr, "error: failed to read file\n"); return 1; }

    channel = arg_int(argc, argv, "-c", 0);

    if (xa_decode_channel(data, len, channel, &pcm, &rate, &nch) != 0) {
        fprintf(stderr, "error: XA decode failed\n");
        free(data);
        return 1;
    }

    {
        int64_t frames = (int64_t)(len / 2336) * 224 / (int64_t)nch;
        if (frames < 1) frames = 1;
        if (nch == 1)
            wav_write_mono(outpath, pcm, frames, rate);
        else
            wav_write_stereo(outpath, pcm, frames, rate);
        printf("wrote %s (%lld frames, %d Hz, %d ch)\n", outpath, (long long)frames, rate, nch);
    }

    free(pcm);
    free(data);
    return 0;
}

static int cmd_xa_inspect(int argc, char **argv)
{
    uint8_t *data;
    size_t len;
    XaStreamInfo streams[32];
    int count, i;

    if (argc < 3) { usage(); return 1; }

    data = read_file(argv[2], &len);
    if (!data) { fprintf(stderr, "error: failed to read file\n"); return 1; }

    count = xa_inspect(data, len, streams, 32);
    printf("XA file: %s (%zu bytes, %d streams)\n", argv[2], len, count);
    for (i = 0; i < count; i++) {
        printf("  stream %d: %d Hz, %d ch, %d frames\n",
               i, streams[i].rate, streams[i].channels, streams[i].frame_count);
    }

    free(data);
    return 0;
}

static int cmd_vab_extract(int argc, char **argv)
{
    uint8_t *vh, *vb;
    size_t vh_len, vb_len;
    const char *outdir;
    VabHeader hdr;
    int i;

    if (argc < 4) { usage(); return 1; }

    outdir = arg_flag(argc, argv, "-o");
    if (!outdir) { fprintf(stderr, "error: -o required\n"); return 1; }

    vh = read_file(argv[2], &vh_len);
    vb = read_file(argv[3], &vb_len);
    if (!vh || !vb) { fprintf(stderr, "error: failed to read files\n"); return 1; }

    if (vab_parse_vh(vh, vh_len, &hdr) != 0) {
        fprintf(stderr, "error: failed to parse VH\n");
        return 1;
    }

    make_dir(outdir);

    for (i = 0; i < (int)hdr.ps_count; i++) {
        int16_t *pcm = NULL;
        int n = vab_decode_vag(vb, vb_len, &hdr, i, &pcm);
        if (n > 0 && pcm) {
            char path[512];
            snprintf(path, sizeof(path), "%s/vag_%03d.wav", outdir, i);
            wav_write_mono(path, pcm, n, 44100);
            printf("  %s (%d frames)\n", path, n);
            free(pcm);
        }
    }

    printf("extracted %d VAGs to %s\n", (int)hdr.ps_count, outdir);
    free(vh);
    free(vb);
    return 0;
}

static int cmd_vab_inspect(int argc, char **argv)
{
    uint8_t *vh;
    size_t vh_len;
    VabHeader hdr;
    int i;

    if (argc < 3) { usage(); return 1; }

    vh = read_file(argv[2], &vh_len);
    if (!vh) { fprintf(stderr, "error: failed to read file\n"); return 1; }

    if (vab_parse_vh(vh, vh_len, &hdr) != 0) {
        fprintf(stderr, "error: failed to parse VH\n");
        free(vh);
        return 1;
    }

    printf("VAB: %s\n", argv[2]);
    printf("  version: %u\n", hdr.version);
    printf("  tones: %u\n", hdr.ps_count);
    printf("  body_size: %u\n", hdr.body_size);

    for (i = 0; i < (int)hdr.ps_count; i++) {
        VabTone *t = &hdr.tones[i];
        printf("  [%3d] prog=%d note=%d-%d center=%d vag_off=%u vag_sz=%u adsr1=0x%02X adsr2=0x%02X\n",
               i, t->prog, t->min_note, t->max_note, t->center_note,
               t->vag_offset, t->vag_size, t->adsr1, t->adsr2);
    }

    free(vh);
    return 0;
}

static int cmd_sep_inspect(int argc, char **argv)
{
    uint8_t *data;
    size_t len;
    SepFile sep;
    int i;

    if (argc < 3) { usage(); return 1; }

    data = read_file(argv[2], &len);
    if (!data) { fprintf(stderr, "error: failed to read file\n"); return 1; }

    if (sep_parse(data, len, &sep) != 0) {
        fprintf(stderr, "error: failed to parse SEP\n");
        free(data);
        return 1;
    }

    printf("SEP: %s (%zu bytes)\n", argv[2], len);
    printf("  sequences: %d\n", sep.sequence_count);

    for (i = 0; i < sep.sequence_count; i++) {
        SepSequence *s = &sep.sequences[i];
        printf("  [%d] resolution=%d events=%d\n",
               i, s->resolution, s->event_count);
    }

    sep_free(&sep);
    free(data);
    return 0;
}

static int cmd_sep2mid(int argc, char **argv)
{
    uint8_t *data;
    size_t len;
    const char *outpath;
    SepFile sep;
    int seq_idx;

    if (argc < 3) { usage(); return 1; }

    outpath = arg_flag(argc, argv, "-o");
    if (!outpath) { fprintf(stderr, "error: -o required\n"); return 1; }

    data = read_file(argv[2], &len);
    if (!data) { fprintf(stderr, "error: failed to read file\n"); return 1; }

    if (sep_parse(data, len, &sep) != 0) {
        fprintf(stderr, "error: failed to parse SEP\n");
        free(data);
        return 1;
    }

    seq_idx = arg_int(argc, argv, "-s", 0);

    if (sep_to_midi(&sep, seq_idx, outpath) != 0) {
        fprintf(stderr, "error: MIDI export failed\n");
        sep_free(&sep);
        free(data);
        return 1;
    }

    printf("wrote %s (sequence %d)\n", outpath, seq_idx);
    sep_free(&sep);
    free(data);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) { usage(); return 1; }

    if (strcmp(argv[1], "play") == 0) return cmd_play(argc, argv);
    if (strcmp(argv[1], "play-xa") == 0) return cmd_play_xa(argc, argv);
    if (strcmp(argv[1], "play-vag") == 0) return cmd_play_vag(argc, argv);
    if (strcmp(argv[1], "render") == 0) return cmd_render(argc, argv);
    if (strcmp(argv[1], "xa-decode") == 0) return cmd_xa_decode(argc, argv);
    if (strcmp(argv[1], "xa-inspect") == 0) return cmd_xa_inspect(argc, argv);
    if (strcmp(argv[1], "vab-extract") == 0) return cmd_vab_extract(argc, argv);
    if (strcmp(argv[1], "vab-inspect") == 0) return cmd_vab_inspect(argc, argv);
    if (strcmp(argv[1], "sep-inspect") == 0) return cmd_sep_inspect(argc, argv);
    if (strcmp(argv[1], "sep2mid") == 0) return cmd_sep2mid(argc, argv);

    fprintf(stderr, "unknown command: %s\n", argv[1]);
    usage();
    return 1;
}
