#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "util.h"

#define SEP_MAGIC 0x70514553u

static uint32_t read_vlq(const uint8_t *p, const uint8_t *end, int *used)
{
    uint32_t v = 0;
    int n = 0;

    while (p + n < end) {
        v = (v << 7) | (p[n] & 0x7F);
        if (!(p[n] & 0x80)) {
            *used = n + 1;
            return v;
        }
        n++;
    }
    *used = n;
    return v;
}

static int midi_data_len(uint8_t status)
{
    uint8_t hi = status & 0xF0;

    if (hi == 0xC0 || hi == 0xD0)
        return 1;
    if (hi >= 0x80 && hi <= 0xE0)
        return 2;
    return 0;
}

static int write_vlq(FILE *f, uint32_t v)
{
    uint8_t buf[5];
    int n = 0, i;

    buf[n++] = v & 0x7F;
    v >>= 7;
    while (v) {
        buf[n++] = (v & 0x7F) | 0x80;
        v >>= 7;
    }
    for (i = n - 1; i >= 0; i--) {
        if (fputc(buf[i], f) == EOF)
            return -1;
    }
    return 0;
}

int sep_parse(const uint8_t *data, size_t len, SepFile *sep)
{
    uint32_t magic;
    uint16_t nseq;
    size_t pos;
    int si;

    memset(sep, 0, sizeof(*sep));

    if (len < 8)
        return -1;

    magic = rd_u32be(data);
    if (magic != SEP_MAGIC)
        return -1;

    nseq = rd_u16be(data + 6);
    if (nseq == 0)
        nseq = 1;
    pos  = 8;

    sep->sequences = calloc(nseq, sizeof(*sep->sequences));
    if (!sep->sequences)
        return -1;
    sep->sequence_count = nseq;

    for (si = 0; si < nseq; si++) {
        SepSequence *seq = &sep->sequences[si];
        uint16_t resolution;
        uint32_t data_size;
        const uint8_t *ev, *ev_end;
        uint8_t running = 0;
        int cap = 64;

        if (pos + 13 > len)
            goto fail;

        resolution = rd_u16be(data + pos);
        data_size  = rd_u32be(data + pos + 7);
        pos += 12;

        if (pos + data_size > len)
            goto fail;

        seq->resolution = resolution;
        seq->events = malloc(cap * sizeof(*seq->events));
        if (!seq->events)
            goto fail;
        seq->event_count = 0;

        ev     = data + pos;
        ev_end = ev + data_size;

        while (ev < ev_end) {
            SepEvent *e;
            uint8_t status;
            int used, dlen;

            uint32_t delta = read_vlq(ev, ev_end, &used);
            ev += used;
            if (ev >= ev_end)
                break;

            if (*ev & 0x80) {
                status = *ev++;
                running = status;
            } else {
                status = running;
            }

            if (seq->event_count >= cap) {
                SepEvent *tmp;
                cap *= 2;
                tmp = realloc(seq->events, cap * sizeof(*tmp));
                if (!tmp)
                    goto fail;
                seq->events = tmp;
            }

            e = &seq->events[seq->event_count];
            memset(e, 0, sizeof(*e));
            e->delta = delta;
            e->type  = status;

            if (status == 0xFF) {
                uint32_t mlen;

                if (ev >= ev_end)
                    break;
                e->meta_type = *ev++;
                mlen = read_vlq(ev, ev_end, &used);
                ev += used;
                e->meta_len = (int)mlen;
                if (mlen > 0) {
                    e->meta = malloc(mlen);
                    if (!e->meta)
                        goto fail;
                    if (ev + mlen > ev_end)
                        mlen = (uint32_t)(ev_end - ev);
                    memcpy(e->meta, ev, mlen);
                    ev += mlen;
                }
            } else {
                dlen = midi_data_len(status);
                if (dlen >= 1 && ev < ev_end)
                    e->data1 = *ev++;
                if (dlen >= 2 && ev < ev_end)
                    e->data2 = *ev++;
            }

            seq->event_count++;
        }

        pos += data_size;
    }

    return 0;

fail:
    sep_free(sep);
    return -1;
}

void sep_free(SepFile *sep)
{
    int i, j;

    if (!sep->sequences)
        return;

    for (i = 0; i < sep->sequence_count; i++) {
        SepSequence *seq = &sep->sequences[i];
        for (j = 0; j < seq->event_count; j++)
            free(seq->events[j].meta);
        free(seq->events);
    }
    free(sep->sequences);
    sep->sequences = NULL;
    sep->sequence_count = 0;
}

int sep_to_midi(const SepFile *sep, int seq_index, const char *path)
{
    FILE *f;
    const SepSequence *seq;
    long trk_start, trk_end;
    uint32_t trk_len;
    int i;

    if (seq_index < 0 || seq_index >= sep->sequence_count)
        return -1;

    seq = &sep->sequences[seq_index];

    f = fopen(path, "wb");
    if (!f)
        return -1;

    fwrite("MThd", 1, 4, f);
    wr_u32be(f, 6);
    wr_u16be(f, 0);
    wr_u16be(f, 1);
    wr_u16be(f, (uint16_t)seq->resolution);

    fwrite("MTrk", 1, 4, f);
    wr_u32be(f, 0);
    trk_start = ftell(f);

    write_vlq(f, 0);
    fputc(0xFF, f);
    fputc(0x51, f);
    fputc(0x03, f);
    fputc(0x07, f);
    fputc(0xA1, f);
    fputc(0x20, f);

    write_vlq(f, 0);
    fputc(0xFF, f);
    fputc(0x58, f);
    fputc(0x04, f);
    fputc(0x04, f);
    fputc(0x02, f);
    fputc(0x18, f);
    fputc(0x08, f);

    for (i = 0; i < seq->event_count; i++) {
        const SepEvent *e = &seq->events[i];

        write_vlq(f, e->delta);

        if (e->type == 0xFF) {
            fputc(0xFF, f);
            fputc(e->meta_type, f);
            write_vlq(f, (uint32_t)e->meta_len);
            if (e->meta_len > 0 && e->meta)
                fwrite(e->meta, 1, e->meta_len, f);
        } else {
            int dlen = midi_data_len(e->type);
            fputc(e->type, f);
            if (dlen >= 1)
                fputc(e->data1, f);
            if (dlen >= 2)
                fputc(e->data2, f);
        }
    }

    write_vlq(f, 0);
    fputc(0xFF, f);
    fputc(0x2F, f);
    fputc(0x00, f);

    trk_end = ftell(f);
    trk_len = (uint32_t)(trk_end - trk_start);
    fseek(f, trk_start - 4, SEEK_SET);
    wr_u32be(f, trk_len);

    fclose(f);
    return 0;
}
