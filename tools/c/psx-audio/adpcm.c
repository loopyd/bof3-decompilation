#include "audio.h"
#include "util.h"

void psx_adpcm_init(PsxAdpcmState* st) {
  st->prev1 = 0;
  st->prev2 = 0;
}

int psx_adpcm_decode_block(const uint8_t block[16], int16_t* out,
                           PsxAdpcmState* st) {
  int     shift = block[0] & 0x0F;
  int     filter = (block[0] >> 4) & 0x0F;
  int     f0, f1, i;
  int32_t prev1 = st->prev1;
  int32_t prev2 = st->prev2;

  if (shift > 12)
    shift = 9;
  if (filter > 4)
    filter = 0;
  f0 = PSX_FILTER_POS[filter];
  f1 = PSX_FILTER_NEG[filter];

  for (i = 0; i < 28; i++) {
    uint8_t byte = block[2 + i / 2];
    int32_t raw_nibble;
    int32_t sample;

    if (i & 1)
      raw_nibble = (byte >> 4) & 0x0F;
    else
      raw_nibble = byte & 0x0F;

    sample = (int32_t)(int16_t)((uint16_t)raw_nibble << 12) >> shift;
    sample += (prev1 * f0) >> 6;
    sample += (prev2 * f1) >> 6;

    if (sample < -32768)
      sample = -32768;
    if (sample > 32767)
      sample = 32767;

    out[i] = (int16_t)sample;
    prev2 = prev1;
    prev1 = sample;
  }

  st->prev1 = (int16_t)prev1;
  st->prev2 = (int16_t)prev2;
  return 28;
}
