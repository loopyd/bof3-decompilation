#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"

int main(void) {
  uint8_t  sector[2336] = {0};
  int16_t* pcm = NULL;
  int      rate;
  int      channels;
  int64_t  frames;
  FILE*    wav;
  uint8_t  header[44];
  int      failed;

  sector[1] = 3; /* channel */
  sector[2] = 4; /* audio submode */
  sector[3] = 0; /* mono, 37800 Hz */
  frames = xa_decode_channel(sector, sizeof(sector), 3, &pcm, &rate, &channels);
  failed = frames != 4032 || rate != 37800 || channels != 1 || !pcm;
  if (!failed) {
    for (int64_t i = 0; i < frames; i++)
      if (pcm[i] != 0)
        failed = 1;
  }
  if (!failed && wav_write_mono("xa_test.wav", pcm, frames, rate) != 0)
    failed = 1;
  free(pcm);
  if (!failed) {
    wav = fopen("xa_test.wav", "rb");
    failed = !wav || fread(header, 1, sizeof(header), wav) != sizeof(header) ||
             memcmp(header, "RIFF", 4) != 0 || memcmp(header + 8, "WAVE", 4) != 0 ||
             header[22] != 1 || header[24] != 0xa8 || header[25] != 0x93;
    if (wav)
      fclose(wav);
  }
  remove("xa_test.wav");
  return failed;
}
