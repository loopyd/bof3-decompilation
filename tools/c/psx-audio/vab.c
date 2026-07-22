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
    uint32_t vag_offsets[256];
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
    if (off + 512 > len || vs > 255)
        return -1;

    vag_offsets[0] = 0;
    for (p = 1; p <= vs; p++)
        vag_offsets[p] = vag_offsets[p - 1] + (uint32_t)rd_u16le(data + off + p * 2) * 8;

    for (p = 0; p < ps && ti < 256; p++) {
        for (t = 0; t < VAB_TONES_PER_PROG && ti < 256; t++) {
            const uint8_t *tn = data + VAB_TONE_OFF +
                                (p * VAB_TONES_PER_PROG + t) * VAB_TONE_SIZE;
            uint8_t vol = tn[2];
            int16_t prog = rd_i16le(tn + 20);
            int16_t vag = rd_i16le(tn + 22);
            if (vag == 0 && vol == 0)
                continue;
            if (vag < 1 || vag > (int)vs)
                continue;
            if (prog < 0 || prog > 127)
                continue;

            hdr->tones[ti].prog        = (uint8_t)prog;
            hdr->tones[ti].min_note    = tn[6];
            hdr->tones[ti].max_note    = tn[7];
            hdr->tones[ti].center_note = tn[4];
            hdr->tones[ti].shift       = tn[5] > 127 ? 127 : tn[5];
            hdr->tones[ti].vol         = tn[2];
            hdr->tones[ti].pan         = tn[3];
            hdr->tones[ti].pitch_bend_min = tn[12];
            hdr->tones[ti].pitch_bend_max = tn[13];
            hdr->tones[ti].adsr1       = rd_u16le(tn + 16);
            hdr->tones[ti].adsr2       = rd_u16le(tn + 18);
            hdr->tones[ti].vag_offset  = vag_offsets[vag - 1];
            hdr->tones[ti].vag_size    = vag_offsets[vag] - vag_offsets[vag - 1];
            ti++;
        }
    }

    hdr->ps_count = ti;
    return 0;
}

int vab_decode_vag(const uint8_t *vb, size_t vb_len, const VabHeader *hdr,
                   int vag_index, int16_t **pcm)
{
    return vab_decode_vag_ex(vb, vb_len, hdr, vag_index, pcm, NULL, NULL);
}

int vab_decode_vag_ex(const uint8_t *vb, size_t vb_len, const VabHeader *hdr,
                      int vag_index, int16_t **pcm,
                      int64_t *loop_start, int64_t *loop_end)
{
    uint32_t start, sz, nblk, decoded_blocks, total_blocks, i;
    uint32_t loop_block = 0, end_block = 0, loop_blocks = 0;
    int has_loop = 0, has_end = 0;
    const uint8_t *src;
    int16_t *out;
    int64_t ls = -1, le = -1;
    PsxAdpcmState st;

    if (vag_index < 0 || vag_index >= (int)hdr->ps_count)
        return -1;

    start = hdr->tones[vag_index].vag_offset;
    sz    = hdr->tones[vag_index].vag_size;

    if (sz <= 16 || start + sz > vb_len)
        return -1;

    src  = vb + start + 16;
    nblk = (sz - 16) / 16;

    decoded_blocks = nblk;
    for (i = 0; i < nblk; i++) {
        uint8_t flags = src[i * 16 + 1];
        if (flags & 0x04) {
            loop_block = i;
            has_loop = 1;
        }
        if (flags & 0x01) {
            has_end = 1;
            decoded_blocks = i + 1;
            end_block = i;
            if (!(flags & 0x02))
                has_loop = 0;
            break;
        }
    }
    if (!has_end)
        has_loop = 0;

    total_blocks = decoded_blocks;
    if (has_loop) {
        loop_blocks = end_block - loop_block + 1;
        total_blocks += loop_blocks * 8;
    }

    out = malloc((size_t)total_blocks * 28 * sizeof(*out));
    if (!out)
        return -1;

    psx_adpcm_init(&st);
    for (i = 0; i < decoded_blocks; i++) {
        uint8_t flags = src[i * 16 + 1];
        psx_adpcm_decode_block(src + i * 16, out + i * 28, &st);
        if (flags & 0x01) {
            le = (int64_t)i * 28 + 28;
            break;
        }
    }

    if (has_loop) {
        uint32_t pass, out_block = decoded_blocks;
        for (pass = 0; pass < 8; pass++) {
            for (i = loop_block; i <= end_block; i++) {
                psx_adpcm_decode_block(src + i * 16, out + out_block * 28, &st);
                out_block++;
            }
        }
        ls = (int64_t)(total_blocks - loop_blocks) * 28;
        le = (int64_t)total_blocks * 28;
    }

    if (loop_start) *loop_start = ls;
    if (loop_end) *loop_end = (le >= 0) ? le : (int64_t)decoded_blocks * 28;

    *pcm = out;
    return (int)(total_blocks * 28);
}
