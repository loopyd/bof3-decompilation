#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "util.h"

#define XA_SECTOR_SIZE    2336
#define XA_UNITS          18
#define XA_UNIT_SIZE      128
#define XA_GROUPS         4
#define XA_GROUP_BYTES    28
#define XA_GROUP_SAMPLES  56
#define XA_MONO_SAMPLES   (XA_GROUPS * XA_GROUP_SAMPLES)
#define XA_STEREO_SAMPLES (2 * XA_GROUP_SAMPLES)
#define XA_DATA_OFF       8

static void xa_decode_group(const uint8_t *d, int16_t *out, int sf,
                            int16_t *p1, int16_t *p2)
{
    int shift  = 12 - (sf & 0x0F);
    int filter = (sf >> 4) & 0x03;
    int fp = PSX_FILTER_POS[filter], fn = PSX_FILTER_NEG[filter];
    int i;

    for (i = 0; i < XA_GROUP_BYTES; i++) {
        int s;

        s = (int)(int16_t)((d[i] & 0x0F) << 12) >> shift;
        s += (*p1 * fp) >> 6;
        s += (*p2 * fn) >> 6;
        if (s > 32767) s = 32767;
        if (s < -32768) s = -32768;
        *p2 = *p1;
        *p1 = (int16_t)s;
        out[i * 2] = (int16_t)s;

        s = (int)(int16_t)(((d[i] >> 4) & 0x0F) << 12) >> shift;
        s += (*p1 * fp) >> 6;
        s += (*p2 * fn) >> 6;
        if (s > 32767) s = 32767;
        if (s < -32768) s = -32768;
        *p2 = *p1;
        *p1 = (int16_t)s;
        out[i * 2 + 1] = (int16_t)s;
    }
}

int xa_inspect(const uint8_t *data, size_t len, XaStreamInfo *streams, int max)
{
    size_t nsec = len / XA_SECTOR_SIZE;
    int count = 0;
    size_t i;

    for (i = 0; i < nsec; i++) {
        const uint8_t *s = data + i * XA_SECTOR_SIZE;
        int stereo, rate, j, found = -1;

        if (!(s[2] & 0x04))
            continue;

        stereo = (s[3] & 0x01) != 0;
        rate   = (s[3] & 0x04) ? 18900 : 37800;

        for (j = 0; j < count; j++) {
            if (streams[j].channels == (stereo ? 2 : 1) &&
                streams[j].rate == rate) {
                found = j;
                break;
            }
        }

        if (found < 0) {
            if (count >= max)
                continue;
            found = count++;
            streams[found].rate        = rate;
            streams[found].channels    = stereo ? 2 : 1;
            streams[found].frame_count = 0;
        }

        streams[found].frame_count += stereo ? XA_STEREO_SAMPLES
                                             : XA_MONO_SAMPLES;
    }
    return count;
}

int xa_decode_channel(const uint8_t *data, size_t len, int channel,
                      int16_t **pcm, int *rate, int *nch)
{
    size_t nsec = len / XA_SECTOR_SIZE;
    int16_t *out = NULL;
    size_t out_frames = 0, out_cap = 0;
    int16_t p1l = 0, p2l = 0, p1r = 0, p2r = 0;
    int stereo = -1;
    size_t i;

    *pcm = NULL;

    for (i = 0; i < nsec; i++) {
        const uint8_t *s = data + i * XA_SECTOR_SIZE;
        int u;

        if (s[1] != channel || !(s[2] & 0x04))
            continue;

        if (stereo < 0)
            stereo = (s[3] & 0x01) != 0;

        for (u = 0; u < XA_UNITS; u++) {
            const uint8_t *unit = s + XA_DATA_OFF + u * XA_UNIT_SIZE;
            const uint8_t *par  = unit;
            const uint8_t *grp  = unit + 16;
            int g;

            if (!stereo) {
                int16_t tmp[XA_MONO_SAMPLES];
                size_t need;
                int sf = par[0];

                for (g = 0; g < XA_GROUPS; g++)
                    xa_decode_group(grp + g * XA_GROUP_BYTES,
                                    tmp + g * XA_GROUP_SAMPLES,
                                    sf, &p1l, &p2l);

                need = out_frames + XA_MONO_SAMPLES;
                if (need > out_cap) {
                    size_t nc = need * 2;
                    int16_t *t = realloc(out, nc * sizeof(*t));
                    if (!t) { free(out); return -1; }
                    out = t;
                    out_cap = nc;
                }
                memcpy(out + out_frames, tmp, XA_MONO_SAMPLES * sizeof(*out));
                out_frames += XA_MONO_SAMPLES;
            } else {
                int16_t tl[XA_STEREO_SAMPLES], tr[XA_STEREO_SAMPLES];
                size_t need;

                for (g = 0; g < XA_GROUPS; g++) {
                    int sf = par[g * 4];
                    if (g & 1)
                        xa_decode_group(grp + g * XA_GROUP_BYTES,
                                        tr + (g >> 1) * XA_GROUP_SAMPLES,
                                        sf, &p1r, &p2r);
                    else
                        xa_decode_group(grp + g * XA_GROUP_BYTES,
                                        tl + (g >> 1) * XA_GROUP_SAMPLES,
                                        sf, &p1l, &p2l);
                }

                need = out_frames + XA_STEREO_SAMPLES;
                if (need > out_cap) {
                    size_t nc = need * 2;
                    int16_t *t = realloc(out, nc * sizeof(*t));
                    if (!t) { free(out); return -1; }
                    out = t;
                    out_cap = nc;
                }
                for (g = 0; g < XA_STEREO_SAMPLES; g++) {
                    out[(out_frames + g) * 2]     = tl[g];
                    out[(out_frames + g) * 2 + 1] = tr[g];
                }
                out_frames += XA_STEREO_SAMPLES;
            }
        }
    }

    if (!out_frames) {
        free(out);
        return -1;
    }

    *pcm = out;
    *rate = stereo ? ((data[3] & 0x04) ? 18900 : 37800) : 37800;
    *nch  = stereo ? 2 : 1;
    return (int)out_frames;
}
