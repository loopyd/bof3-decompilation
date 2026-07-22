#ifndef PSX_AUDIO_SPU_REVERB_H
#define PSX_AUDIO_SPU_REVERB_H

#include <stddef.h>
#include <stdint.h>

typedef struct SpuReverb SpuReverb;

typedef struct SpuReverbOps {
  const char* name;
  SpuReverb* (*create)(uint32_t start_addr, int sample_rate);
  void (*destroy)(SpuReverb* rvb);
  void (*process)(SpuReverb* rvb, const int32_t* input_left,
                  const int32_t* input_right, int32_t* output_left,
                  int32_t* output_right, size_t frames);
  void (*set_register)(SpuReverb* rvb, uint32_t address, uint16_t value);
  uint16_t (*get_register)(const SpuReverb* rvb, uint32_t address);
} SpuReverbOps;

struct SpuReverb {
  const SpuReverbOps* ops;
};

#define PSX_REVERB_REG_BASE  0x1f801dc0u
#define PSX_REVERB_REG_COUNT 32

#define PSX_REVERB_VOLL  0x1f801d84u
#define PSX_REVERB_VOLR  0x1f801d86u
#define PSX_REVERB_START 0x1f801da2u

SpuReverb* spu_reverb_create(const SpuReverbOps* ops, uint32_t start_addr,
                             int sample_rate);
void       spu_reverb_destroy(SpuReverb* rvb);
void       spu_reverb_process(SpuReverb* rvb, const int32_t* input_left,
                              const int32_t* input_right, int32_t* output_left,
                              int32_t* output_right, size_t frames);
void spu_reverb_set_register(SpuReverb* rvb, uint32_t address, uint16_t value);
uint16_t spu_reverb_get_register(const SpuReverb* rvb, uint32_t address);

extern const SpuReverbOps spu_reverb_nocash_ops;

#endif
