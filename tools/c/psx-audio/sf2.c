#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "util.h"

#define SF2_GUARD 46

static void wr_id(FILE *f, const char *id)
{
    fwrite(id, 1, 4, f);
}

static void wr_name(FILE *f, const char *s, size_t n)
{
    char buf[20];
    size_t len;
    memset(buf, 0, n);
    len = strlen(s);
    if (len >= n)
        len = n - 1;
    memcpy(buf, s, len);
    fwrite(buf, 1, n, f);
}

static void wr_info_str(FILE *f, const char *id, const char *s)
{
    size_t len = strlen(s) + 1;
    size_t padded = (len + 1) & ~(size_t)1;
    size_t i;
    wr_id(f, id);
    wr_u32le(f, (uint32_t)padded);
    fwrite(s, 1, len, f);
    for (i = len; i < padded; i++)
        fputc(0, f);
}

static long wr_list_begin(FILE *f, const char *type)
{
    long pos;
    wr_id(f, "LIST");
    pos = ftell(f);
    wr_u32le(f, 0);
    wr_id(f, type);
    return pos;
}

static void wr_list_end(FILE *f, long size_pos)
{
    long end = ftell(f);
    fseek(f, size_pos, SEEK_SET);
    wr_u32le(f, (uint32_t)(end - size_pos - 4));
    fseek(f, end, SEEK_SET);
}

static long wr_chunk_begin(FILE *f, const char *id)
{
    long pos;
    wr_id(f, id);
    pos = ftell(f);
    wr_u32le(f, 0);
    return pos;
}

static void wr_chunk_end(FILE *f, long size_pos)
{
    long end = ftell(f);
    fseek(f, size_pos, SEEK_SET);
    wr_u32le(f, (uint32_t)(end - size_pos - 4));
    fseek(f, end, SEEK_SET);
}

int vab_to_sf2(const uint8_t *vh_data, size_t vh_len,
               const uint8_t *vb_data, size_t vb_len,
               const char *output_path, const char *name)
{
    VabHeader hdr;
    int nt, i, j;
    uint8_t progs[256];
    int nprogs = 0;
    uint32_t vag_offs[256];
    int nvags = 0;
    int tone_sample[256];
    int sample_center[256];
    int16_t *vag_pcm[256];
    int vag_frames[256];
    uint32_t sample_start[256];
    uint32_t smpl_total;
    int prog_tone_count[256];
    FILE *f;
    long riff_pos, info_pos, sdta_pos, smpl_pos, pdta_pos;
    int igen_idx;
    static const int16_t zeros[SF2_GUARD] = {0};

    if (vab_parse_vh(vh_data, vh_len, &hdr) != 0)
        return -1;

    nt = (int)hdr.tone_count;
    if (nt == 0)
        return -1;

    memset(vag_pcm, 0, sizeof(vag_pcm));
    memset(vag_frames, 0, sizeof(vag_frames));
    memset(prog_tone_count, 0, sizeof(prog_tone_count));

    for (i = 0; i < nt; i++) {
        int found = 0;
        for (j = 0; j < nprogs; j++) {
            if (progs[j] == hdr.tones[i].prog) {
                found = 1;
                break;
            }
        }
        if (!found)
            progs[nprogs++] = hdr.tones[i].prog;
    }

    for (i = 0; i < nt; i++) {
        int found = -1;
        for (j = 0; j < nvags; j++) {
            if (vag_offs[j] == hdr.tones[i].vag_offset) {
                found = j;
                break;
            }
        }
        if (found < 0) {
            vag_offs[nvags] = hdr.tones[i].vag_offset;
            sample_center[nvags] = hdr.tones[i].center_note;
            found = nvags++;
        }
        tone_sample[i] = found;
    }

    for (i = 0; i < nt; i++) {
        int si = tone_sample[i];
        int frames;
        if (vag_pcm[si])
            continue;
        frames = vab_decode_vag(vb_data, vb_len, &hdr, i, &vag_pcm[si]);
        if (frames <= 0)
            goto fail;
        vag_frames[si] = frames;
    }

    smpl_total = 0;
    for (j = 0; j < nvags; j++) {
        sample_start[j] = smpl_total;
        smpl_total += (uint32_t)vag_frames[j] + 2 * SF2_GUARD;
    }

    for (i = 0; i < nt; i++) {
        for (j = 0; j < nprogs; j++) {
            if (hdr.tones[i].prog == progs[j]) {
                prog_tone_count[j]++;
                break;
            }
        }
    }

    f = fopen(output_path, "wb");
    if (!f)
        goto fail;

    wr_id(f, "RIFF");
    riff_pos = ftell(f);
    wr_u32le(f, 0);
    wr_id(f, "sfbk");

    info_pos = wr_list_begin(f, "INFO");
    wr_id(f, "ifil");
    wr_u32le(f, 4);
    wr_u16le(f, 2);
    wr_u16le(f, 1);
    wr_info_str(f, "isng", "EMU8000");
    wr_info_str(f, "INAM", name ? name : "VAB Bank");
    wr_list_end(f, info_pos);

    sdta_pos = wr_list_begin(f, "sdta");
    smpl_pos = wr_chunk_begin(f, "smpl");
    for (j = 0; j < nvags; j++) {
        fwrite(zeros, sizeof(int16_t), SF2_GUARD, f);
        fwrite(vag_pcm[j], sizeof(int16_t), (size_t)vag_frames[j], f);
        fwrite(zeros, sizeof(int16_t), SF2_GUARD, f);
    }
    wr_chunk_end(f, smpl_pos);
    wr_list_end(f, sdta_pos);

    pdta_pos = wr_list_begin(f, "pdta");

    wr_id(f, "phdr");
    wr_u32le(f, (uint32_t)(nprogs + 1) * 38);
    {
        uint16_t bag = 0;
        for (j = 0; j < nprogs; j++) {
            char pname[20];
            snprintf(pname, sizeof(pname), "P%03d", progs[j]);
            wr_name(f, pname, 20);
            wr_u16le(f, (uint16_t)progs[j]);
            wr_u16le(f, 0);
            wr_u16le(f, bag);
            wr_u32le(f, 0);
            wr_u32le(f, 0);
            wr_u32le(f, 0);
            bag += 1;
        }
        wr_name(f, "EOP", 20);
        wr_u16le(f, 0);
        wr_u16le(f, 0);
        wr_u16le(f, bag);
        wr_u32le(f, 0);
        wr_u32le(f, 0);
        wr_u32le(f, 0);
    }

    wr_id(f, "pbag");
    wr_u32le(f, (uint32_t)(nprogs + 1) * 4);
    for (j = 0; j < nprogs; j++) {
        wr_u16le(f, (uint16_t)j);
        wr_u16le(f, 0);
    }
    wr_u16le(f, (uint16_t)nprogs);
    wr_u16le(f, 0);

    wr_id(f, "pmod");
    wr_u32le(f, 10);
    for (i = 0; i < 5; i++)
        wr_u16le(f, 0);

    wr_id(f, "pgen");
    wr_u32le(f, (uint32_t)(nprogs + 1) * 4);
    for (j = 0; j < nprogs; j++) {
        wr_u16le(f, 41);
        wr_u16le(f, (uint16_t)j);
    }
    wr_u16le(f, 0);
    wr_u16le(f, 0);

    wr_id(f, "inst");
    wr_u32le(f, (uint32_t)(nprogs + 1) * 22);
    {
        uint16_t ibag = 0;
        for (j = 0; j < nprogs; j++) {
            char iname[20];
            snprintf(iname, sizeof(iname), "I%03d", progs[j]);
            wr_name(f, iname, 20);
            wr_u16le(f, ibag);
            ibag += (uint16_t)prog_tone_count[j];
        }
        wr_name(f, "EOI", 20);
        wr_u16le(f, ibag);
    }

    wr_id(f, "ibag");
    wr_u32le(f, (uint32_t)(nt + 1) * 4);
    igen_idx = 0;
    for (i = 0; i < nt; i++) {
        wr_u16le(f, (uint16_t)igen_idx);
        wr_u16le(f, 0);
        igen_idx += 3;
    }
    wr_u16le(f, (uint16_t)igen_idx);
    wr_u16le(f, 0);

    wr_id(f, "imod");
    wr_u32le(f, 10);
    for (i = 0; i < 5; i++)
        wr_u16le(f, 0);

    wr_id(f, "igen");
    wr_u32le(f, (uint32_t)(nt * 3 + 1) * 4);
    for (i = 0; i < nt; i++) {
        wr_u16le(f, 43);
        wr_u16le(f, (uint16_t)(hdr.tones[i].min_note |
                                (hdr.tones[i].max_note << 8)));
        wr_u16le(f, 54);
        wr_u16le(f, 1);
        wr_u16le(f, 53);
        wr_u16le(f, (uint16_t)tone_sample[i]);
    }
    wr_u16le(f, 0);
    wr_u16le(f, 0);

    wr_id(f, "shdr");
    wr_u32le(f, (uint32_t)(nvags + 1) * 46);
    for (j = 0; j < nvags; j++) {
        char sname[20];
        uint32_t st = sample_start[j];
        uint32_t en = st + (uint32_t)vag_frames[j] + 2 * SF2_GUARD;
        snprintf(sname, sizeof(sname), "VAG%03d", j);
        wr_name(f, sname, 20);
        wr_u32le(f, st);
        wr_u32le(f, en);
        wr_u32le(f, st + SF2_GUARD);
        wr_u32le(f, en - SF2_GUARD);
        wr_u32le(f, 44100);
        fputc(sample_center[j], f);
        fputc(0, f);
        wr_u16le(f, 0);
        wr_u16le(f, 1);
    }
    wr_name(f, "EOS", 20);
    wr_u32le(f, 0);
    wr_u32le(f, 0);
    wr_u32le(f, 0);
    wr_u32le(f, 0);
    wr_u32le(f, 0);
    fputc(0, f);
    fputc(0, f);
    wr_u16le(f, 0);
    wr_u16le(f, 0);

    wr_list_end(f, pdta_pos);

    {
        long end = ftell(f);
        fseek(f, riff_pos, SEEK_SET);
        wr_u32le(f, (uint32_t)(end - riff_pos - 4));
        fseek(f, end, SEEK_SET);
    }

    fclose(f);
    for (j = 0; j < nvags; j++)
        free(vag_pcm[j]);
    return 0;

fail:
    for (j = 0; j < nvags; j++)
        free(vag_pcm[j]);
    return -1;
}
