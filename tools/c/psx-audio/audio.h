#ifndef PSX_AUDIO_H
#define PSX_AUDIO_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
  int16_t* pcm;
  int64_t  frames;
  int      rate;
} RenderOutput;

typedef struct {
  uint32_t vh_size;
  uint32_t vb_size;
  uint32_t declared_file_size;
  uint16_t program_count;
  uint16_t tone_count;
  uint16_t vag_count;
  uint16_t sequence_count;
  uint32_t remapped_tones;
  uint32_t missing_note_events;
  uint32_t layered_note_events;
  uint32_t bad_vag_ranges;
  uint32_t bad_sample_prefixes;
  uint32_t samples_without_end;
  uint32_t reverb_tones;
  uint32_t modulation_tones;
  uint32_t bend_lsb_events;
  uint32_t ignored_control_events;
  uint32_t loop_control_events;
  uint32_t missing_notes[128][128];
} AudioAuditReport;

typedef struct {
  int rate;
  int channels;
  int frame_count;
} XaStreamInfo;

typedef struct {
  uint8_t  prog;
  uint8_t  storage_block;
  uint8_t  tone_slot;
  uint8_t  program_vol;
  uint8_t  program_pan;
  uint8_t  program_priority;
  uint8_t  priority;
  uint8_t  min_note;
  uint8_t  max_note;
  uint8_t  center_note;
  uint8_t  shift;
  uint8_t  mode;
  uint8_t  vibrato_width;
  uint8_t  vibrato_time;
  uint8_t  portamento_width;
  uint8_t  portamento_time;
  uint8_t  vol;
  uint8_t  pan;
  uint8_t  pitch_bend_min;
  uint8_t  pitch_bend_max;
  uint16_t adsr1;
  uint16_t adsr2;
  uint32_t vag_size;
  uint32_t vag_offset;
} VagAtr;

typedef struct {
  uint8_t tones;
  uint8_t mvol;
  uint8_t mpan;
  uint8_t attr;
  uint8_t reserve[4];
} ProgAtr;

typedef struct {
  int16_t left;
  int16_t right;
} SndVolume;

typedef enum {
  SPU_REVERB_OFF = 0,
  SPU_REVERB_ROOM,
  SPU_REVERB_STUDIO_A,
  SPU_REVERB_STUDIO_B,
  SPU_REVERB_STUDIO_C,
  SPU_REVERB_HALL,
  SPU_REVERB_SPACE,
  SPU_REVERB_ECHO,
  SPU_REVERB_DELAY,
  SPU_REVERB_PIPE,
  SPU_REVERB_COUNT
} SpuReverbMode;

typedef struct {
  SpuReverbMode mode;
  SndVolume     depth;
  int           delay;
  int           feedback;
} SpuReverbAttr;

typedef struct {
  uint32_t version;
  uint32_t file_size;
  uint16_t program_count;
  uint16_t declared_tone_count;
  uint16_t vag_count;
  uint16_t tone_count;
  uint8_t  master_vol;
  uint8_t  master_pan;
  uint8_t  program_tone_count[128];
  VagAtr   tones[2048];
} VabHdr;

typedef struct {
  uint32_t delta;
  uint8_t  type;
  uint8_t  data1;
  uint8_t  data2;
  uint8_t  meta_type;
  uint8_t* meta;
  int      meta_len;
} SepEvent;

typedef struct {
  int       seq_id;
  int       resolution;
  int       tempo_us;
  int       time_num;
  int       time_den;
  SepEvent* events;
  int       event_count;
} SepSequence;

typedef struct {
  SepSequence* sequences;
  int          sequence_count;
} SepFile;

typedef struct {
  int      phase;
  int      level;
  uint16_t adsr1;
  uint16_t adsr2;
  int      attack_rate;
  int      decay_rate;
  int      sustain_rate;
  int      release_rate;
  int      sustain_level;
  int32_t  counter;
  int32_t  counter_inc;
  int32_t  step;
  int      decreasing;
  int      exponential;
} SpuAdsr;

typedef struct {
  int16_t prev1;
  int16_t prev2;
} PsxAdpcmState;

void psx_adpcm_init(PsxAdpcmState* st);
int  psx_adpcm_decode_block(const uint8_t block[16], int16_t* out,
                            PsxAdpcmState* st);

int render_bgm(const uint8_t* sep_data, size_t sep_len, const uint8_t* vh_data,
               size_t vh_len, const uint8_t* vb_data, size_t vb_len,
               int seq_index, int output_rate, RenderOutput* out);
int audio_audit_bgm(const uint8_t* vh_data, size_t vh_len,
                    const uint8_t* vb_data, size_t vb_len,
                    const uint8_t* sep_data, size_t sep_len,
                    AudioAuditReport* report);

int xa_decode_channel(const uint8_t* data, size_t len, int channel,
                      int16_t** pcm, int* rate, int* nch);
int xa_inspect(const uint8_t* data, size_t len, XaStreamInfo* streams, int max);

int vab_parse_vh(const uint8_t* data, size_t len, VabHdr* hdr);
int vab_decode_vag(const uint8_t* vb, size_t vb_len, const VabHdr* hdr,
                   int vag_index, int16_t** pcm);
int vab_decode_vag_ex(const uint8_t* vb, size_t vb_len, const VabHdr* hdr,
                      int vag_index, int16_t** pcm, int64_t* loop_start,
                      int64_t* loop_end);

int  sep_parse(const uint8_t* data, size_t len, SepFile* sep);
int  sep_to_midi(const SepFile* sep, int seq_index, const char* path);
void sep_free(SepFile* sep);

void     spu_adsr_key_on(SpuAdsr* adsr, uint16_t adsr1, uint16_t adsr2);
void     spu_adsr_key_off(SpuAdsr* adsr);
int      spu_adsr_tick(SpuAdsr* adsr);
uint16_t spu_pitch_from_note(int note, int fine, int center, int shift);

int wav_write_mono(const char* path, const int16_t* pcm, int64_t count,
                   int rate);
int wav_write_stereo(const char* path, const int16_t* pcm, int64_t frames,
                     int rate);
int ogg_write_stereo(const char* path, const int16_t* pcm, int64_t frames,
                     int rate);
int flac_write_stereo(const char* path, const int16_t* pcm, int64_t frames,
                      int rate);

int vab_to_sf2(const uint8_t* vh_data, size_t vh_len, const uint8_t* vb_data,
               size_t vb_len, const char* output_path, const char* name);

#endif
