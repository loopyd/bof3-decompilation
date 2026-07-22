#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "audio.h"
#include "util.h"

typedef struct {
    int16_t *samples;
    int64_t len;
    int64_t loop_start;
    int64_t loop_end;
} VagCache;

typedef struct {
    int16_t *samples;
    int64_t len;
    int64_t loop_start;
    int64_t loop_end;
    double pos;
    SpuAdsr adsr;
    float volume;
    float pan;
    int active;
    int channel;
    int note;
    int center_note;
    int center_shift;
    double pitch_factor;
    int bend;
    int pitch_bend_min;
    int pitch_bend_max;
} Voice;

#define MAX_VOICES 64

typedef struct {
    int channel;
    int note;
    int velocity;
    int64_t frame;
    int is_on;
    int program;
    float volume;
    float pan;
    int bend;
    int is_bend;
} NoteEvent;

typedef struct {
    int program;
    float volume;
    float pan;
    int bend;
} ChannelState;

/* PSX SPU pan: 0..127, 64 = center. At center both channels get full volume. */
static void pan_gains(int pan_psx, float *l, float *r)
{
    if (pan_psx < 0) pan_psx = 0;
    if (pan_psx > 127) pan_psx = 127;
    *l = (pan_psx < 64) ? 1.0f : (float)(127 - pan_psx) / 64.0f;
    *r = (pan_psx > 64) ? 1.0f : (float)pan_psx / 64.0f;
}

static double voice_pitch(const Voice *v, int bend)
{
    int bend_value = bend - 64;
    int note = v->note;
    int fine = 0;
    int fine_index;
    int semitone;
    int octave;
    int table_index;
    int pitch;

    if (bend_value < 0) {
        int scaled = bend_value * v->pitch_bend_min;
        note += scaled / 64 - 1;
        fine = 2 * (scaled % 64) + 127;
    } else if (bend_value > 0) {
        int scaled = bend_value * v->pitch_bend_max;
        note += scaled / 63;
        fine = 2 * (scaled % 63);
    }

    fine_index = fine + v->center_shift;
    if (fine_index < 0)
        fine_index += 7;
    fine_index >>= 3;

    semitone = 0;
    if (fine_index > 15) {
        semitone = 1;
        fine_index -= 16;
    }

    semitone += note - (v->center_note - 60);
    table_index = 16 * (semitone % 12) + fine_index;
    octave = semitone / 12 - 5;
    pitch = (int)(4096.0 * pow(2.0, (double)table_index / 192.0));
    if (octave > 0)
        pitch <<= octave;
    else if (octave < 0)
        pitch >>= -octave;
    if (pitch > 0x4000)
        pitch = 0x4000;
    return (double)pitch / 4096.0;
}

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
        channels[ch].pan = 64.0f;
        channels[ch].bend = 64;
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
        int64_t ls, le;
        int n = vab_decode_vag_ex(vb_data, vb_len, &vhdr, i, &pcm, &ls, &le);
        if (n > 0 && pcm) {
            vag_cache[i].samples = pcm;
            vag_cache[i].len = n;
            vag_cache[i].loop_start = ls;
            vag_cache[i].loop_end = le;
        }
    }

    {
        SepSequence *seq = &sep.sequences[seq_index];
        int resolution = seq->resolution > 0 ? seq->resolution : 48;
        double tempo_us = seq->tempo_us > 0 ? (double)seq->tempo_us : 500000.0;
        double elapsed_seconds = 0.0;
        int64_t tempo_tick = 0;
        int64_t tick = 0;

        for (i = 0; i < (int)seq->event_count; i++) {
            SepEvent *ev = &seq->events[i];
            tick += ev->delta;

            if (ev->type == 0xFF && ev->meta_type == 0x51 && ev->meta_len >= 3) {
                elapsed_seconds += (double)(tick - tempo_tick) * tempo_us /
                                   (1000000.0 * resolution);
                tempo_tick = tick;
                tempo_us = (double)((ev->meta[0] << 16) | (ev->meta[1] << 8) | ev->meta[2]);
                continue;
            }

            if ((ev->type & 0xF0) == 0xC0) {
                channels[ev->type & 0x0F].program = ev->data1;
                continue;
            }

            if ((ev->type & 0xF0) == 0xB0) {
                if (ev->data1 == 7)
                    channels[ev->type & 0x0F].volume = ev->data2 / 127.0f;
                else if (ev->data1 == 10)
                    channels[ev->type & 0x0F].pan = (float)ev->data2;
                continue;
            }

            if ((ev->type & 0xF0) == 0xE0) {
                int channel = ev->type & 0x0F;
                double seconds = elapsed_seconds + (double)(tick - tempo_tick) * tempo_us /
                                                    (1000000.0 * resolution);
                channels[channel].bend = ev->data2;
                if (event_count >= event_cap) {
                    event_cap = event_cap ? event_cap * 2 : 1024;
                    events = (NoteEvent *)realloc(events, (size_t)event_cap * sizeof(NoteEvent));
                }
                memset(&events[event_count], 0, sizeof(events[event_count]));
                events[event_count].channel = channel;
                events[event_count].frame = (int64_t)(seconds * output_rate);
                events[event_count].bend = channels[channel].bend;
                events[event_count].is_bend = 1;
                event_count++;
                continue;
            }

            if ((ev->type & 0xF0) == 0x90 && ev->data2 > 0) {
                double seconds = elapsed_seconds + (double)(tick - tempo_tick) * tempo_us /
                                                    (1000000.0 * resolution);
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
                events[event_count].is_bend = 0;
                events[event_count].program = channels[ev->type & 0x0F].program;
                events[event_count].volume = channels[ev->type & 0x0F].volume;
                events[event_count].pan = channels[ev->type & 0x0F].pan;
                events[event_count].bend = channels[ev->type & 0x0F].bend;
                event_count++;
            } else if (((ev->type & 0xF0) == 0x80) ||
                       ((ev->type & 0xF0) == 0x90 && ev->data2 == 0)) {
                double seconds = elapsed_seconds + (double)(tick - tempo_tick) * tempo_us /
                                                    (1000000.0 * resolution);
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
                events[event_count].is_bend = 0;
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
        free(mix_l); free(mix_r); free(events);
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
                if (ne->is_bend) {
                    for (i = 0; i < MAX_VOICES; i++) {
                        if (voices[i].active && voices[i].channel == ne->channel)
                            voices[i].bend = ne->bend;
                    }
                } else if (ne->is_on) {
                    int prog = ne->program;
                    int matching = 0;
                    int fallback = -1;
                    int best_dist = 999;

                    for (i = 0; i < (int)vhdr.ps_count; i++) {
                        VabTone *t = &vhdr.tones[i];
                        if (t->prog == prog &&
                            ne->note >= t->min_note &&
                            ne->note <= t->max_note)
                            matching++;
                        if (t->prog == prog) {
                            int dist = abs(ne->note - t->center_note);
                            if (dist < best_dist) {
                                best_dist = dist;
                                fallback = i;
                            }
                        }
                    }

                    for (j = 0; j < (int)vhdr.ps_count; j++) {
                        VabTone *t = &vhdr.tones[j];
                        Voice *v;
                        int slot = -1;
                        float eff_pan;

                        if (matching > 0) {
                            if (t->prog != prog || ne->note < t->min_note ||
                                ne->note > t->max_note)
                                continue;
                        } else if (j != fallback) {
                            continue;
                        }
                        if (!vag_cache[j].samples)
                            continue;

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
                        if (slot < 0)
                            break;

                        v = &voices[slot];
                        v->samples = vag_cache[j].samples;
                        v->len = vag_cache[j].len;
                        v->loop_start = vag_cache[j].loop_start;
                        v->loop_end = vag_cache[j].loop_end;
                        v->pos = 0.0;
                        v->volume = ne->volume *
                                    (ne->velocity / 127.0f) *
                                    (t->vol / 127.0f);
                        eff_pan = (float)t->pan + (ne->pan - 64.0f);
                        if (eff_pan < 0.0f) eff_pan = 0.0f;
                        if (eff_pan > 127.0f) eff_pan = 127.0f;
                        v->pan = eff_pan;
                        v->active = 1;
                        v->channel = ne->channel;
                        v->note = ne->note;
                        v->center_note = t->center_note;
                        v->center_shift = t->shift;
                        v->bend = ne->bend;
                        v->pitch_bend_min = t->pitch_bend_min;
                        v->pitch_bend_max = t->pitch_bend_max;
                        v->pitch_factor = voice_pitch(v, v->bend);
                        memset(&v->adsr, 0, sizeof(SpuAdsr));
                        spu_adsr_key_on(&v->adsr, t->adsr1, t->adsr2);
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
                double s;
                float lg, rg;
                int64_t idx;
                int frac;
                double src_pos;

                if (!v->active)
                    continue;

                level = spu_adsr_tick(&v->adsr);
                if (v->adsr.phase >= 4) {
                    v->active = 0;
                    continue;
                }

                v->pitch_factor = voice_pitch(v, v->bend);

                src_pos = v->pos;
                idx = (int64_t)src_pos;

                if (v->loop_start >= 0 && idx >= v->loop_end) {
                    int64_t loop_len = v->loop_end - v->loop_start;
                    if (loop_len > 0) {
                        src_pos = (double)v->loop_start +
                                  fmod(src_pos - (double)v->loop_end, (double)loop_len);
                        v->pos = src_pos;
                        idx = (int64_t)src_pos;
                    } else {
                        v->active = 0;
                        continue;
                    }
                } else if (idx >= v->len) {
                    v->active = 0;
                    continue;
                }

                frac = (int)((src_pos - (double)idx) * 256.0);
                if (frac > 255) frac = 255;

                sample = psx_gauss_interp(v->samples, v->len, idx, frac);
                v->pos += v->pitch_factor;

                s = (double)sample * ((double)level / 32767.0) * (double)v->volume;
                pan_gains((int)v->pan, &lg, &rg);

                if (frame < total_frames) {
                    mix_l[frame] += s * lg;
                    mix_r[frame] += s * rg;
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
            free(mix_l); free(mix_r); free(events);
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
