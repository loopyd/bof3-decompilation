#ifndef PSX_AUDIO_H
#define PSX_AUDIO_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    int16_t *pcm;
    int64_t frames;
    int rate;
} RenderOutput;

typedef struct {
    int rate;
    int channels;
    int frame_count;
} XaStreamInfo;

typedef struct {
    uint8_t prog;
    uint8_t min_note;
    uint8_t max_note;
    uint8_t center_note;
    uint16_t adsr1;
    uint16_t adsr2;
    uint16_t vag_size;
    uint16_t vag_offset;
} VabTone;

typedef struct {
    uint32_t version;
    uint32_t ps_count;
    uint32_t body_size;
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
    int resolution;
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

int xa_decode_channel(const uint8_t *data, size_t len, int channel,
                      int16_t **pcm, int *rate, int *nch);
int xa_inspect(const uint8_t *data, size_t len, XaStreamInfo *streams, int max);

int vab_parse_vh(const uint8_t *data, size_t len, VabHeader *hdr);
int vab_decode_vag(const uint8_t *vb, size_t vb_len, const VabHeader *hdr,
                   int vag_index, int16_t **pcm);

int sep_parse(const uint8_t *data, size_t len, SepFile *sep);
int sep_to_midi(const SepFile *sep, int seq_index, const char *path);
void sep_free(SepFile *sep);

void spu_adsr_key_on(SpuAdsr *adsr, uint16_t adsr1, uint16_t adsr2);
void spu_adsr_key_off(SpuAdsr *adsr);
int spu_adsr_tick(SpuAdsr *adsr);

int wav_write_mono(const char *path, const int16_t *pcm, int64_t count, int rate);
int wav_write_stereo(const char *path, const int16_t *pcm, int64_t frames, int rate);

/* --- EMI container --- */

#define EMI_MAX_ENTRIES 64
#define EMI_TYPE_VH  6
#define EMI_TYPE_VB  7
#define EMI_TYPE_AUX 8
#define EMI_TYPE_SEQ 10

typedef struct {
    uint32_t size;
    uint32_t offset;
    uint16_t type;
} EmiEntry;

typedef struct {
    int count;
    EmiEntry entries[EMI_MAX_ENTRIES];
    const uint8_t *data;
    size_t data_len;
} EmiFile;

int emi_parse(const uint8_t *data, size_t len, EmiFile *emi);
const uint8_t *emi_find_type(const EmiFile *emi, int type, uint32_t *size);

#endif
