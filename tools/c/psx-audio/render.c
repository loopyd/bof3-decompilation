#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "audio.h"
#include "util.h"

typedef struct {
    int16_t *samples;
    int64_t len;
} VagCache;

typedef struct {
    int16_t *samples;
    int64_t len;
    int64_t pos;
    SpuAdsr adsr;
    float volume;
    float pan;
    int active;
    int channel;
    int note;
    double pitch_factor;
} Voice;

#define MAX_VOICES 64

typedef struct {
    int channel;
    int note;
    int velocity;
    int64_t frame;
    int is_on;
} NoteEvent;

typedef struct {
    int program;
    float volume;
    float pan;
} ChannelState;

int render_bgm(const uint8_t *sep_data, size_t sep_len,
               const uint8_t *vh_data, size_t vh_len,
               const uint8_t *vb_data, size_t vb_len,
               int seq_index, int output_rate, RenderOutput *out)
{
    SepFile sep;
    VabHeader vhdr;
    VagCache *vag_cache = NULL;
    Voice voices[MAX_VOICES];
    ChannelState channels[16];
    NoteEvent *events = NULL;
    int event_count = 0, event_cap = 0;
    double *mix_l = NULL, *mix_r = NULL;
    int64_t total_frames = 0;
    int i, j, ch;

    if (!sep_data || !vh_data || !vb_data || !out)
        return -1;

    memset(out, 0, sizeof(*out));
    memset(voices, 0, sizeof(voices));

    for (ch = 0; ch < 16; ch++) {
        channels[ch].program = 0;
        channels[ch].volume = 1.0f;
        channels[ch].pan = 0.5f;
    }

    if (sep_parse(sep_data, sep_len, &sep) != 0)
        return -1;

    if (seq_index < 0 || seq_index >= (int)sep.sequence_count) {
        sep_free(&sep);
        return -1;
    }

    if (vab_parse_vh(vh_data, vh_len, &vhdr) != 0) {
        sep_free(&sep);
        return -1;
    }

    vag_cache = (VagCache *)calloc(vhdr.ps_count, sizeof(VagCache));
    if (!vag_cache) {
        sep_free(&sep);
        return -1;
    }

    for (i = 0; i < (int)vhdr.ps_count; i++) {
        int16_t *pcm = NULL;
        int n = vab_decode_vag(vb_data, vb_len, &vhdr, i, &pcm);
        if (n > 0 && pcm) {
            vag_cache[i].samples = pcm;
            vag_cache[i].len = n;
        }
    }

    {
        SepSequence *seq = &sep.sequences[seq_index];
        int resolution = seq->resolution > 0 ? seq->resolution : 480;
        double tempo_us = 500000.0;
        int64_t tick = 0;

        for (i = 0; i < (int)seq->event_count; i++) {
            SepEvent *ev = &seq->events[i];
            tick += ev->delta;

            if (ev->type == 0xFF && ev->meta_type == 0x51 && ev->meta_len >= 3) {
                tempo_us = (double)((ev->meta[0] << 16) | (ev->meta[1] << 8) | ev->meta[2]);
                continue;
            }

            if ((ev->type & 0xF0) == 0xC0) {
                channels[ev->type & 0x0F].program = ev->data1;
                continue;
            }

            if (ev->type == 0xB0) {
                if (ev->data1 == 7)
                    channels[ev->type & 0x0F].volume = ev->data2 / 127.0f;
                else if (ev->data1 == 10)
                    channels[ev->type & 0x0F].pan = ev->data2 / 127.0f;
                continue;
            }

            if ((ev->type & 0xF0) == 0x90 && ev->data2 > 0) {
                double seconds = (double)tick * tempo_us / (1000000.0 * resolution);
                int64_t frame = (int64_t)(seconds * output_rate);
                if (event_count >= event_cap) {
                    event_cap = event_cap ? event_cap * 2 : 1024;
                    events = (NoteEvent *)realloc(events, (size_t)event_cap * sizeof(NoteEvent));
                }
                events[event_count].channel = ev->type & 0x0F;
                events[event_count].note = ev->data1;
                events[event_count].velocity = ev->data2;
                events[event_count].frame = frame;
                events[event_count].is_on = 1;
                event_count++;
            } else if (((ev->type & 0xF0) == 0x80) ||
                       ((ev->type & 0xF0) == 0x90 && ev->data2 == 0)) {
                double seconds = (double)tick * tempo_us / (1000000.0 * resolution);
                int64_t frame = (int64_t)(seconds * output_rate);
                if (event_count >= event_cap) {
                    event_cap = event_cap ? event_cap * 2 : 1024;
                    events = (NoteEvent *)realloc(events, (size_t)event_cap * sizeof(NoteEvent));
                }
                events[event_count].channel = ev->type & 0x0F;
                events[event_count].note = ev->data1;
                events[event_count].velocity = 0;
                events[event_count].frame = frame;
                events[event_count].is_on = 0;
                event_count++;
            }
        }
    }

    if (event_count == 0) {
        free(events);
        for (i = 0; i < (int)vhdr.ps_count; i++)
            free(vag_cache[i].samples);
        free(vag_cache);
        sep_free(&sep);
        return -1;
    }

    total_frames = events[event_count - 1].frame + (int64_t)output_rate * 2;

    mix_l = (double *)calloc((size_t)total_frames, sizeof(double));
    mix_r = (double *)calloc((size_t)total_frames, sizeof(double));
    if (!mix_l || !mix_r) {
        free(mix_l);
        free(mix_r);
        free(events);
        for (i = 0; i < (int)vhdr.ps_count; i++)
            free(vag_cache[i].samples);
        free(vag_cache);
        sep_free(&sep);
        return -1;
    }

    {
        int ev_idx = 0;
        int64_t frame;

        for (frame = 0; frame < total_frames; frame++) {
            while (ev_idx < event_count && events[ev_idx].frame <= frame) {
                NoteEvent *ne = &events[ev_idx];
                if (ne->is_on) {
                    int slot = -1;
                    int prog = channels[ne->channel].program;
                    int best_vag = -1;
                    int best_dist = 999;

                    for (i = 0; i < (int)vhdr.ps_count; i++) {
                        VabTone *t = &vhdr.tones[i];
                        if (t->prog == prog &&
                            ne->note >= t->min_note &&
                            ne->note <= t->max_note) {
                            int dist = abs(ne->note - t->center_note);
                            if (dist < best_dist) {
                                best_dist = dist;
                                best_vag = i;
                            }
                        }
                    }

                    if (best_vag < 0) {
                        for (i = 0; i < (int)vhdr.ps_count; i++) {
                            VabTone *t = &vhdr.tones[i];
                            if (t->prog == prog) {
                                int dist = abs(ne->note - t->center_note);
                                if (dist < best_dist) {
                                    best_dist = dist;
                                    best_vag = i;
                                }
                            }
                        }
                    }

                    if (best_vag >= 0 && vag_cache[best_vag].samples) {
                        for (i = 0; i < MAX_VOICES; i++) {
                            if (!voices[i].active) {
                                slot = i;
                                break;
                            }
                        }
                        if (slot < 0) {
                            for (i = 0; i < MAX_VOICES; i++) {
                                if (voices[i].channel == ne->channel &&
                                    voices[i].note == ne->note) {
                                    slot = i;
                                    break;
                                }
                            }
                        }
                        if (slot >= 0) {
                            VabTone *t = &vhdr.tones[best_vag];
                            Voice *v = &voices[slot];
                            double semitones = (double)(ne->note - t->center_note);
                            v->samples = vag_cache[best_vag].samples;
                            v->len = vag_cache[best_vag].len;
                            v->pos = 0;
                            v->pitch_factor = pow(2.0, semitones / 12.0);
                            v->volume = channels[ne->channel].volume *
                                        (ne->velocity / 127.0f);
                            v->pan = channels[ne->channel].pan;
                            v->active = 1;
                            v->channel = ne->channel;
                            v->note = ne->note;
                            memset(&v->adsr, 0, sizeof(SpuAdsr));
                            spu_adsr_key_on(&v->adsr, t->adsr1, t->adsr2);
                        }
                    }
                } else {
                    for (i = 0; i < MAX_VOICES; i++) {
                        if (voices[i].active &&
                            voices[i].channel == ne->channel &&
                            voices[i].note == ne->note) {
                            spu_adsr_key_off(&voices[i].adsr);
                        }
                    }
                }
                ev_idx++;
            }

            for (i = 0; i < MAX_VOICES; i++) {
                Voice *v = &voices[i];
                int32_t sample;
                int level;
                double s, sl, sr;
                int64_t idx;
                int frac;
                double src_pos;

                if (!v->active)
                    continue;

                level = spu_adsr_tick(&v->adsr);
                if (level == 0 && v->adsr.phase >= 4) {
                    v->active = 0;
                    continue;
                }

                src_pos = (double)v->pos * v->pitch_factor;
                idx = (int64_t)src_pos;
                frac = (int)((src_pos - (double)idx) * 256.0);
                if (frac > 255) frac = 255;

                if (idx >= v->len) {
                    v->active = 0;
                    continue;
                }

                sample = psx_gauss_interp(v->samples, v->len, idx, frac);
                v->pos++;

                s = (double)sample * ((double)level / 32767.0) * (double)v->volume;
                sl = s * (1.0 - (double)v->pan);
                sr = s * (double)v->pan;

                if (frame < total_frames) {
                    mix_l[frame] += sl;
                    mix_r[frame] += sr;
                }
            }
        }
    }

    {
        double peak = 0.0;
        int16_t *pcm;

        for (i = 0; i < (int64_t)total_frames; i++) {
            if (fabs(mix_l[i]) > peak) peak = fabs(mix_l[i]);
            if (fabs(mix_r[i]) > peak) peak = fabs(mix_r[i]);
        }

        pcm = (int16_t *)malloc((size_t)total_frames * 2 * sizeof(int16_t));
        if (!pcm) {
            free(mix_l);
            free(mix_r);
            free(events);
            for (i = 0; i < (int)vhdr.ps_count; i++)
                free(vag_cache[i].samples);
            free(vag_cache);
            sep_free(&sep);
            return -1;
        }

        {
            double scale = (peak > 30000.0) ? (30000.0 / peak) : 1.0;
            for (i = 0; i < (int64_t)total_frames; i++) {
                double l = mix_l[i] * scale;
                double r = mix_r[i] * scale;
                if (l > 32767.0) l = 32767.0;
                if (l < -32768.0) l = -32768.0;
                if (r > 32767.0) r = 32767.0;
                if (r < -32768.0) r = -32768.0;
                pcm[i * 2] = (int16_t)l;
                pcm[i * 2 + 1] = (int16_t)r;
            }
        }

        out->pcm = pcm;
        out->frames = total_frames;
        out->rate = output_rate;
    }

    free(mix_l);
    free(mix_r);
    free(events);
    for (i = 0; i < (int)vhdr.ps_count; i++)
        free(vag_cache[i].samples);
    free(vag_cache);
    sep_free(&sep);

    return 0;
}
