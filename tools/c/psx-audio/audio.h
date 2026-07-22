#ifndef PSX_AUDIO_H
#define PSX_AUDIO_H

#include <stdint.h>
#include <stddef.h>

#include "psf.h"

typedef struct {
    int16_t *pcm;
    int64_t frames;
    int rate;
} RenderOutput;

typedef enum {
    AUDIO_ENGINE_FAST = 0,
    AUDIO_ENGINE_GAME = 1
} AudioEngine;

typedef enum {
    AUDIO_STATUS_OK = 0,
    AUDIO_STATUS_INVALID_ARGUMENT,
    AUDIO_STATUS_UNSUPPORTED_ENGINE,
    AUDIO_STATUS_RENDER_FAILED
} AudioStatus;

typedef struct {
    AudioEngine engine;
    const uint8_t *sep_data;
    size_t sep_len;
    const uint8_t *vh_data;
    size_t vh_len;
    const uint8_t *vb_data;
    size_t vb_len;
    int sequence;
    int output_rate;
    const Psf1Image *game_image;
} AudioRenderRequest;

typedef struct {
    RenderOutput audio;
} AudioRenderResult;

typedef struct {
    int rate;
    int channels;
    int frame_count;
} XaStreamInfo;

typedef struct {
    uint8_t prog;
    uint8_t program_vol;
    uint8_t program_pan;
    uint8_t min_note;
    uint8_t max_note;
    uint8_t center_note;
    uint8_t shift;
    uint8_t mode;
    uint8_t vibrato_width;
    uint8_t vibrato_time;
    uint8_t portamento_width;
    uint8_t portamento_time;
    uint8_t vol;
    uint8_t pan;
    uint8_t pitch_bend_min;
    uint8_t pitch_bend_max;
    uint16_t adsr1;
    uint16_t adsr2;
    uint32_t vag_size;
    uint32_t vag_offset;
} VabTone;

typedef struct {
    uint32_t version;
    uint32_t ps_count;
    uint32_t body_size;
    uint8_t master_vol;
    uint8_t master_pan;
    VabTone tones[256];
} VabHeader;

typedef struct {
    uint32_t delta;
    uint8_t type;
    uint8_t data1;
    uint8_t data2;
    uint8_t meta_type;
    uint8_t *meta;
    int meta_len;
} SepEvent;

typedef struct {
    int seq_id;
    int resolution;
    int tempo_us;
    int time_num;
    int time_den;
    SepEvent *events;
    int event_count;
} SepSequence;

typedef struct {
    SepSequence *sequences;
    int sequence_count;
} SepFile;

typedef struct {
    int phase;
    int level;
    uint16_t adsr1;
    uint16_t adsr2;
    int attack_rate;
    int decay_rate;
    int sustain_rate;
    int release_rate;
    int sustain_level;
    int32_t counter;
    int32_t counter_inc;
    int32_t step;
    int decreasing;
    int exponential;
} SpuAdsr;

typedef struct {
    int16_t prev1;
    int16_t prev2;
} PsxAdpcmState;

void psx_adpcm_init(PsxAdpcmState *st);
int psx_adpcm_decode_block(const uint8_t block[16], int16_t *out, PsxAdpcmState *st);

int render_bgm(const uint8_t *sep_data, size_t sep_len,
               const uint8_t *vh_data, size_t vh_len,
               const uint8_t *vb_data, size_t vb_len,
               int seq_index, int output_rate, RenderOutput *out);
int render_game_bgm(const AudioRenderRequest *request, RenderOutput *out);
AudioStatus audio_render(const AudioRenderRequest *request,
                         AudioRenderResult *result);
const char *audio_status_string(AudioStatus status);

int xa_decode_channel(const uint8_t *data, size_t len, int channel,
                      int16_t **pcm, int *rate, int *nch);
int xa_inspect(const uint8_t *data, size_t len, XaStreamInfo *streams, int max);

int vab_parse_vh(const uint8_t *data, size_t len, VabHeader *hdr);
int vab_decode_vag(const uint8_t *vb, size_t vb_len, const VabHeader *hdr,
                   int vag_index, int16_t **pcm);
int vab_decode_vag_ex(const uint8_t *vb, size_t vb_len, const VabHeader *hdr,
                      int vag_index, int16_t **pcm,
                      int64_t *loop_start, int64_t *loop_end);

int sep_parse(const uint8_t *data, size_t len, SepFile *sep);
int sep_to_midi(const SepFile *sep, int seq_index, const char *path);
void sep_free(SepFile *sep);

void spu_adsr_key_on(SpuAdsr *adsr, uint16_t adsr1, uint16_t adsr2);
void spu_adsr_key_off(SpuAdsr *adsr);
int spu_adsr_tick(SpuAdsr *adsr);
uint16_t spu_pitch_from_note(int note, int fine, int center, int shift);

int wav_write_mono(const char *path, const int16_t *pcm, int64_t count, int rate);
int wav_write_stereo(const char *path, const int16_t *pcm, int64_t frames, int rate);
int ogg_write_stereo(const char *path, const int16_t *pcm, int64_t frames, int rate);
int flac_write_stereo(const char *path, const int16_t *pcm, int64_t frames, int rate);

int vab_to_sf2(const uint8_t *vh_data, size_t vh_len,
               const uint8_t *vb_data, size_t vb_len,
               const char *output_path, const char *name);

#endif
