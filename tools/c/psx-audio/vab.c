#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "util.h"

#define VAB_MAGIC     0x56414270u
#define VAB_TONE_OFF  0x820
#define VAB_TONE_SIZE 32
#define VAB_TONES_PER_PROG 16

int vab_parse_vh(const uint8_t *data, size_t len, VabHeader *hdr)
{
    uint32_t ps, vs;
    size_t off;
    uint32_t ti = 0;
    uint32_t p, t;

    if (len < VAB_TONE_OFF)
        return -1;
    if (rd_u32le(data) != VAB_MAGIC)
        return -1;

    hdr->version   = rd_u32le(data + 4);
    hdr->body_size = rd_u32le(data + 0x0C);
    ps = rd_u16le(data + 0x12);
    vs = rd_u16le(data + 0x16);
    hdr->ps_count = ps;

    off = VAB_TONE_OFF + (size_t)ps * VAB_TONES_PER_PROG * VAB_TONE_SIZE;

    for (p = 0; p < ps && ti < 256; p++) {
        for (t = 0; t < VAB_TONES_PER_PROG && ti < 256; t++) {
            const uint8_t *tn = data + VAB_TONE_OFF +
                                (p * VAB_TONES_PER_PROG + t) * VAB_TONE_SIZE;
            uint8_t vol = tn[2];
            int16_t vag = rd_i16le(tn + 22);
            uint16_t vo, ve;

            if (vag == 0 && vol == 0)
                continue;
            if (vag < 1 || vag > (int)vs)
                continue;

            vo = rd_u16le(data + off + (vag - 1) * 2);
            ve = rd_u16le(data + off + vag * 2);

            hdr->tones[ti].prog        = (uint8_t)p;
            hdr->tones[ti].min_note    = tn[6];
            hdr->tones[ti].max_note    = tn[7];
            hdr->tones[ti].center_note = tn[4];
            hdr->tones[ti].adsr1       = rd_u16le(tn + 16);
            hdr->tones[ti].adsr2       = rd_u16le(tn + 18);
            hdr->tones[ti].vag_offset  = (uint16_t)(vo * 8);
            hdr->tones[ti].vag_size    = (uint16_t)((ve - vo) * 8);
            ti++;
        }
    }

    hdr->ps_count = ti;
    return 0;
}

int vab_decode_vag(const uint8_t *vb, size_t vb_len, const VabHeader *hdr,
                   int vag_index, int16_t **pcm)
{
    uint32_t start, sz, nblk, i;
    const uint8_t *src;
    int16_t *out;
    PsxAdpcmState st;

    if (vag_index < 0 || vag_index >= (int)hdr->ps_count)
        return -1;

    start = hdr->tones[vag_index].vag_offset;
    sz    = hdr->tones[vag_index].vag_size;

    if (sz <= 16 || start + sz > vb_len)
        return -1;

    src  = vb + start + 16;
    nblk = (sz - 16) / 16;

    out = malloc(nblk * 28 * sizeof(*out));
    if (!out)
        return -1;

    psx_adpcm_init(&st);
    for (i = 0; i < nblk; i++)
        psx_adpcm_decode_block(src + i * 16, out + i * 28, &st);

    *pcm = out;
    return (int)(nblk * 28);
}
