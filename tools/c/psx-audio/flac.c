#include <stdlib.h>
#include "audio.h"

#ifdef HAVE_FLAC
#include <FLAC/stream_encoder.h>

int flac_write_stereo(const char* path, const int16_t* pcm, int64_t frames,
                      int rate) {
  FLAC__StreamEncoder* encoder;
  FLAC__int32*         buffer;
  int64_t              pos = 0;
  int                  rc = -1;

  encoder = FLAC__stream_encoder_new();
  if (!encoder)
    return -1;
  buffer = malloc(2048 * sizeof(*buffer));
  if (!buffer)
    goto done;

  if (!FLAC__stream_encoder_set_channels(encoder, 2) ||
      !FLAC__stream_encoder_set_bits_per_sample(encoder, 16) ||
      !FLAC__stream_encoder_set_sample_rate(encoder, (unsigned)rate) ||
      !FLAC__stream_encoder_set_compression_level(encoder, 5) ||
      !FLAC__stream_encoder_set_total_samples_estimate(encoder,
                                                       (FLAC__uint64)frames) ||
      FLAC__stream_encoder_init_file(encoder, path, NULL, NULL) !=
          FLAC__STREAM_ENCODER_INIT_STATUS_OK)
    goto free_buffer;

  while (pos < frames) {
    unsigned count = (frames - pos > 1024) ? 1024u : (unsigned)(frames - pos);
    unsigned i;

    for (i = 0; i < count * 2; i++)
      buffer[i] = pcm[pos * 2 + i];
    if (!FLAC__stream_encoder_process_interleaved(encoder, buffer, count))
      goto finish;
    pos += count;
  }
  rc = 0;

finish:
  if (!FLAC__stream_encoder_finish(encoder))
    rc = -1;
free_buffer:
  free(buffer);
done:
  FLAC__stream_encoder_delete(encoder);
  return rc;
}

#else

int flac_write_stereo(const char* path, const int16_t* pcm, int64_t frames,
                      int rate) {
  (void)path;
  (void)pcm;
  (void)frames;
  (void)rate;
  return -1;
}

#endif
