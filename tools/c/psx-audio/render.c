#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "spu_device.h"
#include "spu_reverb.h"
#include "util.h"

typedef struct {
  int      active;
  int      channel;
  int      note;
  int      center_note;
  int      center_shift;
  uint16_t pitch;
  int      bend;
  int      pitch_bend_min;
  int      pitch_bend_max;
  int      priority;
  uint64_t generation;
  int      seq_idx;
} Voice;

#define MAX_VOICES    24
#define MAX_SEQUENCES 8

typedef struct {
  int     channel;
  int     note;
  int     velocity;
  int64_t frame;
  int     is_on;
  int     program;
  float   volume;
  float   pan;
  int     bend;
  int     is_bend;
  int     seq_idx;
} NoteEvent;

typedef struct {
  int   program;
  float volume;
  float pan;
  int   bend;
} ChannelState;

/* PSX SPU pan: 0..127, 64 = center. At center both channels get full volume. */
static void pan_gains(int pan_psx, float* l, float* r) {
  if (pan_psx < 0)
    pan_psx = 0;
  if (pan_psx > 127)
    pan_psx = 127;
  *l = (pan_psx < 64) ? 1.0f : (float)(127 - pan_psx) / 64.0f;
  *r = (pan_psx > 64) ? 1.0f : (float)pan_psx / 64.0f;
}

static uint16_t voice_pitch(const Voice* v, int bend) {
  int      bend_value = bend - 64;
  int      note = v->note;
  int      fine = 0;
  uint16_t pitch;

  if (bend_value < 0) {
    int scaled = bend_value * v->pitch_bend_min;
    note += scaled / 64 - 1;
    fine = 2 * (scaled % 64) + 127;
  } else if (bend_value > 0) {
    int scaled = bend_value * v->pitch_bend_max;
    note += scaled / 63;
    fine = 2 * (scaled % 63);
  }

  pitch = spu_pitch_from_note(note, fine, v->center_note, v->center_shift);
  if (pitch > 0x4000)
    pitch = 0x4000;
  return pitch;
}

static uint32_t voice_register_address(int voice, uint32_t offset) {
  return PSX_SPU_REGISTER_BASE + (uint32_t)voice * PSX_SPU_VOICE_STRIDE +
         offset;
}

static int write_voice_register(PsxSpu* spu, int voice, uint32_t offset,
                                uint16_t value) {
  return psx_spu_write16(spu, voice_register_address(voice, offset), value, 0);
}

static int write_key_mask(PsxSpu* spu, int voice, int key_on) {
  uint32_t address;
  uint16_t mask = (uint16_t)(1u << (voice & 15));

  if (key_on)
    address = voice < 16 ? PSX_SPU_KEY_ON_LOW : PSX_SPU_KEY_ON_HIGH;
  else
    address = voice < 16 ? PSX_SPU_KEY_OFF_LOW : PSX_SPU_KEY_OFF_HIGH;
  return psx_spu_write16(spu, address, mask, 0);
}

static uint16_t fixed_voice_volume(float gain) {
  if (gain <= 0.0f)
    return 0;
  if (gain >= 1.0f)
    return 0x3fff;
  return (uint16_t)(gain * gain * 16383.0f);
}

/* Diagnostic: set PSX_AUDIO_TRACE=1 to log one line per keyed-on voice so the
   renderer's tone/pitch choices can be diffed against the linked libsnd. */
static int trace_enabled(void) {
  static int cached = -1;
  if (cached < 0) {
    const char* env = getenv("PSX_AUDIO_TRACE");
    cached = (env && env[0] != '\0' && env[0] != '0') ? 1 : 0;
  }
  return cached;
}

static int compare_events(const void* a, const void* b) {
  const NoteEvent* ea = (const NoteEvent*)a;
  const NoteEvent* eb = (const NoteEvent*)b;
  if (ea->frame != eb->frame)
    return (ea->frame < eb->frame) ? -1 : 1;
  {
    int ea_order = ea->is_bend ? 0 : (ea->is_on ? 1 : 2);
    int eb_order = eb->is_bend ? 0 : (eb->is_on ? 1 : 2);
    return ea_order - eb_order;
  }
}

static void build_seq_events(SepSequence* seq, int seq_idx, NoteEvent** events,
                             int* event_count, int* event_cap,
                             ChannelState* channels, int output_rate) {
  int     resolution = seq->resolution > 0 ? seq->resolution : 48;
  double  tempo_us = seq->tempo_us > 0 ? (double)seq->tempo_us : 500000.0;
  double  elapsed_seconds = 0.0;
  int64_t tempo_tick = 0;
  int64_t tick = 0;
  int     i, ch, expanded_ch;
  double  seconds;
  int64_t frame;

  for (i = 0; i < (int)seq->event_count; i++) {
    SepEvent* ev = &seq->events[i];
    tick += ev->delta;

    if (ev->type == 0xFF && ev->meta_type == 0x51 && ev->meta_len >= 3) {
      elapsed_seconds +=
          (double)(tick - tempo_tick) * tempo_us / (1000000.0 * resolution);
      tempo_tick = tick;
      tempo_us =
          (double)((ev->meta[0] << 16) | (ev->meta[1] << 8) | ev->meta[2]);
      continue;
    }

    ch = ev->type & 0x0F;
    expanded_ch = seq_idx * 16 + ch;

    if ((ev->type & 0xF0) == 0xC0) {
      channels[expanded_ch].program = ev->data1;
      continue;
    }
    if ((ev->type & 0xF0) == 0xB0) {
      if (ev->data1 == 7)
        channels[expanded_ch].volume = ev->data2 / 127.0f;
      else if (ev->data1 == 10)
        channels[expanded_ch].pan = (float)ev->data2;
      continue;
    }

    seconds = elapsed_seconds +
              (double)(tick - tempo_tick) * tempo_us / (1000000.0 * resolution);
    frame = (int64_t)(seconds * output_rate);

    if ((ev->type & 0xF0) == 0xE0) {
      channels[expanded_ch].bend = ev->data2;
      if (*event_count >= *event_cap) {
        *event_cap = *event_cap ? *event_cap * 2 : 1024;
        *events = (NoteEvent*)realloc(*events,
                                      (size_t)*event_cap * sizeof(NoteEvent));
      }
      memset(&(*events)[*event_count], 0, sizeof(NoteEvent));
      (*events)[*event_count].channel = ch;
      (*events)[*event_count].frame = frame;
      (*events)[*event_count].bend = channels[expanded_ch].bend;
      (*events)[*event_count].is_bend = 1;
      (*events)[*event_count].seq_idx = seq_idx;
      (*event_count)++;
      continue;
    }

    if ((ev->type & 0xF0) == 0x90 && ev->data2 > 0) {
      if (*event_count >= *event_cap) {
        *event_cap = *event_cap ? *event_cap * 2 : 1024;
        *events = (NoteEvent*)realloc(*events,
                                      (size_t)*event_cap * sizeof(NoteEvent));
      }
      memset(&(*events)[*event_count], 0, sizeof(NoteEvent));
      (*events)[*event_count].channel = ch;
      (*events)[*event_count].note = ev->data1;
      (*events)[*event_count].velocity = ev->data2;
      (*events)[*event_count].frame = frame;
      (*events)[*event_count].is_on = 1;
      (*events)[*event_count].program = channels[expanded_ch].program;
      (*events)[*event_count].volume = channels[expanded_ch].volume;
      (*events)[*event_count].pan = channels[expanded_ch].pan;
      (*events)[*event_count].bend = channels[expanded_ch].bend;
      (*events)[*event_count].seq_idx = seq_idx;
      (*event_count)++;
      continue;
    }

    if ((ev->type & 0xF0) == 0x80 ||
        ((ev->type & 0xF0) == 0x90 && ev->data2 == 0)) {
      if (*event_count >= *event_cap) {
        *event_cap = *event_cap ? *event_cap * 2 : 1024;
        *events = (NoteEvent*)realloc(*events,
                                      (size_t)*event_cap * sizeof(NoteEvent));
      }
      memset(&(*events)[*event_count], 0, sizeof(NoteEvent));
      (*events)[*event_count].channel = ch;
      (*events)[*event_count].note = ev->data1;
      (*events)[*event_count].velocity = 0;
      (*events)[*event_count].frame = frame;
      (*events)[*event_count].is_on = 0;
      (*events)[*event_count].seq_idx = seq_idx;
      (*event_count)++;
    }
  }
}

int render_bgm(const uint8_t* sep_data, size_t sep_len, const uint8_t* vh_data,
               size_t vh_len, const uint8_t* vb_data, size_t vb_len,
               int seq_index, int output_rate, RenderOutput* out) {
  SepFile      sep;
  VabHdr       vhdr;
  PsxSpu*      spu = NULL;
  Voice        voices[MAX_VOICES];
  ChannelState channels[16];
  NoteEvent*   events = NULL;
  int          event_count = 0, event_cap = 0;
  uint64_t     next_generation = 1;
  int16_t*     pcm = NULL;
  int64_t      total_frames = 0;
  int          i, j, ch;

  if (!sep_data || !vh_data || !vb_data || !out || output_rate != 44100 ||
      vb_len > PSX_SPU_RAM_SIZE)
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

  if (seq_index < -1 || seq_index >= (int)sep.sequence_count) {
    sep_free(&sep);
    return -1;
  }

  if (vab_parse_vh(vh_data, vh_len, &vhdr) != 0) {
    sep_free(&sep);
    return -1;
  }

  spu = psx_spu_create();
  if (!spu) {
    sep_free(&sep);
    return -1;
  }
  if (psx_spu_write16(spu, PSX_SPU_TRANSFER_ADDRESS, 0, 0) != 0 ||
      psx_spu_dma_write(spu, vb_data, vb_len, 0) != 0 ||
      psx_spu_write16(spu, PSX_SPU_MAIN_VOLUME_LEFT, 0x3fff, 0) != 0 ||
      psx_spu_write16(spu, PSX_SPU_MAIN_VOLUME_RIGHT, 0x3fff, 0) != 0) {
    psx_spu_destroy(spu);
    sep_free(&sep);
    return -1;
  }

  {
    /* psx-spx Studio Small reverb preset (size=1F40h) */
    static const uint16_t reverb_regs[32] = {
        0x00E3, 0x00A9, 0x6F60, 0x4FA8, 0xBCE0, 0x4510, 0xBEF0, 0xA680,
        0x5680, 0x52C0, 0x0DFB, 0x0B58, 0x0D09, 0x0A3C, 0x0BD9, 0x0973,
        0x0B59, 0x08DA, 0x08D9, 0x05E9, 0x07EC, 0x04B0, 0x06EF, 0x03D2,
        0x05EA, 0x031D, 0x031C, 0x0238, 0x0154, 0x00AA, 0x8000, 0x8000};
    int has_reverb = 0;
    int ri;

    for (ri = 0; ri < (int)vhdr.tone_count; ri++) {
      if (vhdr.tones[ri].mode & 4) {
        has_reverb = 1;
        break;
      }
    }

    if (has_reverb) {
      uint32_t reverb_start = PSX_REVERB_AREA_START;
      for (ri = 0; ri < 32; ri++) {
        uint32_t addr = PSX_REVERB_REG_BASE + (uint32_t)ri * 2;
        if (psx_spu_write16(spu, addr, reverb_regs[ri], 0) != 0)
          break;
      }
      psx_spu_write16(spu, PSX_REVERB_VOLL, 0x3fff, 0);
      psx_spu_write16(spu, PSX_REVERB_VOLR, 0x3fff, 0);
      psx_spu_write16(spu, PSX_REVERB_START, (uint16_t)(reverb_start >> 3), 0);
      psx_spu_write16(spu, PSX_SPU_CONTROL, 0x0080, 0);
    }
  }

  {
    ChannelState channels[MAX_SEQUENCES * 16];
    int          si;

    for (i = 0; i < MAX_SEQUENCES * 16; i++) {
      channels[i].program = 0;
      channels[i].volume = 1.0f;
      channels[i].pan = 64.0f;
      channels[i].bend = 64;
    }

    if (seq_index == -1) {
      for (si = 0; si < (int)sep.sequence_count && si < MAX_SEQUENCES; si++) {
        if (sep.sequences[si].event_count > 0)
          build_seq_events(&sep.sequences[si], si, &events, &event_count,
                           &event_cap, channels, output_rate);
      }
    } else {
      build_seq_events(&sep.sequences[seq_index], 0, &events, &event_count,
                       &event_cap, channels, output_rate);
    }

    if (seq_index == -1 && sep.sequence_count > 1 && event_count > 1)
      qsort(events, (size_t)event_count, sizeof(NoteEvent), compare_events);
  }

  if (event_count == 0) {
    free(events);
    psx_spu_destroy(spu);
    sep_free(&sep);
    return -1;
  }

  total_frames = events[event_count - 1].frame + (int64_t)output_rate * 2;

  pcm = (int16_t*)calloc((size_t)total_frames * 2, sizeof(*pcm));
  if (!pcm) {
    free(events);
    psx_spu_destroy(spu);
    sep_free(&sep);
    return -1;
  }

  {
    int     ev_idx = 0;
    int64_t frame = 0;
    int     render_failed = 0;

    while (frame < total_frames && !render_failed) {
      while (ev_idx < event_count && events[ev_idx].frame <= frame) {
        NoteEvent* ne = &events[ev_idx];
        if (ne->is_bend) {
          for (i = 0; i < MAX_VOICES; i++) {
            if (voices[i].active && voices[i].channel == ne->channel &&
                voices[i].seq_idx == ne->seq_idx) {
              voices[i].bend = ne->bend;
              voices[i].pitch = voice_pitch(&voices[i], ne->bend);
              if (write_voice_register(spu, i, PSX_SPU_VOICE_PITCH,
                                       voices[i].pitch) != 0)
                render_failed = 1;
            }
          }
        } else if (ne->is_on) {
          int prog = ne->program;
          int matching = 0;

          for (i = 0; i < (int)vhdr.tone_count; i++) {
            VagAtr* t = &vhdr.tones[i];
            if (t->prog == prog && ne->note >= t->min_note &&
                ne->note <= t->max_note)
              matching++;
          }

          for (j = 0; j < (int)vhdr.tone_count; j++) {
            VagAtr* t = &vhdr.tones[j];
            Voice*  v;
            int     slot = -1;
            float   eff_pan;
            float   volume;
            float   left_gain, right_gain;

            if (matching == 0 || t->prog != prog || ne->note < t->min_note ||
                ne->note > t->max_note) {
              continue;
            }

            for (i = 0; i < MAX_VOICES; i++) {
              if (voices[i].active &&
                  !psx_spu_voice_active(spu, (unsigned int)i))
                voices[i].active = 0;
              if (!voices[i].active) {
                slot = i;
                break;
              }
            }
            if (slot < 0) {
              int      lowest_priority = 256;
              uint16_t smallest_envelope = UINT16_MAX;
              uint64_t oldest = UINT64_MAX;
              for (i = 0; i < MAX_VOICES; i++) {
                uint16_t envelope = 0;
                psx_spu_read16(
                    spu, voice_register_address(i, PSX_SPU_VOICE_ADSR_VOLUME),
                    &envelope);
                if (voices[i].priority < lowest_priority ||
                    (voices[i].priority == lowest_priority &&
                     envelope < smallest_envelope) ||
                    (voices[i].priority == lowest_priority &&
                     envelope == smallest_envelope &&
                     voices[i].generation < oldest)) {
                  lowest_priority = voices[i].priority;
                  smallest_envelope = envelope;
                  oldest = voices[i].generation;
                  slot = i;
                }
              }
            }
            if (slot < 0)
              break;

            v = &voices[slot];
            volume = ne->volume * (ne->velocity / 127.0f) *
                     (vhdr.master_vol / 127.0f) * (t->program_vol / 127.0f) *
                     (t->vol / 127.0f);
            eff_pan = (float)t->pan + ((float)t->program_pan - 64.0f) +
                      ((float)vhdr.master_pan - 64.0f) + (ne->pan - 64.0f);
            if (eff_pan < 0.0f)
              eff_pan = 0.0f;
            if (eff_pan > 127.0f)
              eff_pan = 127.0f;
            pan_gains((int)eff_pan, &left_gain, &right_gain);
            v->active = 1;
            v->channel = ne->channel;
            v->seq_idx = ne->seq_idx;
            v->note = ne->note;
            v->center_note = t->center_note;
            v->center_shift = t->shift;
            /* libsnd note-on uses base pitch (fine = 0). Pitch bend
                           is applied only by bend events to already-active voices
                           (_SsVmPitchBend -> _SsVmPBVoice), so the channel's stored
                           bend must not be folded into a newly keyed-on note. */
            v->bend = 64;
            v->pitch_bend_min = t->pitch_bend_min;
            v->pitch_bend_max = t->pitch_bend_max;
            v->priority = t->program_priority + t->priority;
            v->generation = next_generation++;
            v->pitch = voice_pitch(v, v->bend);
            if (trace_enabled())
              fprintf(stderr,
                      "note-on frame=%lld seq=%d ch=%d "
                      "prog=%d note=%d "
                      "tone=%d/%d center=%d shift=%d pb=%d/%d "
                      "pitch=0x%04x vag=%u\n",
                      (long long)ne->frame, ne->seq_idx, ne->channel,
                      ne->program, ne->note, t->storage_block, t->tone_slot,
                      t->center_note, t->shift, t->pitch_bend_min,
                      t->pitch_bend_max, v->pitch, t->vag_offset);
            if (write_voice_register(spu, slot, PSX_SPU_VOICE_VOLUME_LEFT,
                                     fixed_voice_volume(volume * left_gain)) !=
                    0 ||
                write_voice_register(spu, slot, PSX_SPU_VOICE_VOLUME_RIGHT,
                                     fixed_voice_volume(volume * right_gain)) !=
                    0 ||
                write_voice_register(spu, slot, PSX_SPU_VOICE_PITCH,
                                     v->pitch) != 0 ||
                write_voice_register(spu, slot, PSX_SPU_VOICE_START_ADDRESS,
                                     (uint16_t)(t->vag_offset >> 3)) != 0 ||
                write_voice_register(spu, slot, PSX_SPU_VOICE_ADSR1,
                                     t->adsr1) != 0 ||
                write_voice_register(spu, slot, PSX_SPU_VOICE_ADSR2,
                                     t->adsr2) != 0 ||
                write_voice_register(spu, slot, PSX_SPU_VOICE_REPEAT_ADDRESS,
                                     (uint16_t)(t->vag_offset >> 3)) != 0 ||
                write_key_mask(spu, slot, 1) != 0)
              render_failed = 1;
            if (!render_failed && (t->mode & 4)) {
              uint32_t rvb_addr =
                  slot < 16 ? PSX_SPU_REVERB_ON_LOW : PSX_SPU_REVERB_ON_HIGH;
              uint16_t rvb_mask = (uint16_t)(1u << (slot & 15));
              uint16_t rvb_cur = 0;
              psx_spu_read16(spu, rvb_addr, &rvb_cur);
              psx_spu_write16(spu, rvb_addr, rvb_cur | rvb_mask, 0);
            }
          }
        } else {
          for (i = 0; i < MAX_VOICES; i++) {
            if (voices[i].active && voices[i].channel == ne->channel &&
                voices[i].note == ne->note &&
                voices[i].seq_idx == ne->seq_idx) {
              if (write_key_mask(spu, i, 0) != 0)
                render_failed = 1;
            }
          }
        }
        ev_idx++;
      }
      if (!render_failed) {
        int64_t next_frame =
            ev_idx < event_count ? events[ev_idx].frame : total_frames;
        if (next_frame > total_frames)
          next_frame = total_frames;
        if (next_frame <= frame)
          next_frame = frame + 1;
        if (psx_spu_render(spu, pcm + frame * 2,
                           (size_t)(next_frame - frame)) != 0)
          render_failed = 1;
        frame = next_frame;
      }
    }
    if (render_failed) {
      free(pcm);
      free(events);
      psx_spu_destroy(spu);
      sep_free(&sep);
      return -1;
    }
  }

  out->pcm = pcm;
  out->frames = total_frames;
  out->rate = output_rate;
  free(events);
  psx_spu_destroy(spu);
  sep_free(&sep);

  return 0;
}
