#include <stdio.h>
#include "audio.h"

#ifdef HAVE_VORBIS
#include <ogg/ogg.h>
#include <vorbis/vorbisenc.h>

static int write_pages(FILE* f, ogg_stream_state* stream, int flush) {
  ogg_page page;
  int      result;

  while ((result = flush ? ogg_stream_flush(stream, &page)
                         : ogg_stream_pageout(stream, &page)) != 0) {
    if (fwrite(page.header, 1, (size_t)page.header_len, f) !=
            (size_t)page.header_len ||
        fwrite(page.body, 1, (size_t)page.body_len, f) != (size_t)page.body_len)
      return -1;
  }
  return 0;
}

int ogg_write_stereo(const char* path, const int16_t* pcm, int64_t frames,
                     int rate) {
  vorbis_info      info;
  vorbis_comment   comment;
  vorbis_dsp_state dsp;
  vorbis_block     block;
  ogg_stream_state stream;
  ogg_packet       packet, header, header_comment, header_code;
  FILE*            f;
  int64_t          pos = 0;
  int              rc = -1;

  f = fopen(path, "wb");
  if (!f)
    return -1;

  vorbis_info_init(&info);
  if (vorbis_encode_init_vbr(&info, 2, rate, 0.4f) != 0)
    goto clear_info;
  vorbis_comment_init(&comment);
  vorbis_comment_add_tag(&comment, "ENCODER", "bof3-audio");
  if (vorbis_analysis_init(&dsp, &info) != 0)
    goto clear_comment;
  if (vorbis_block_init(&dsp, &block) != 0)
    goto clear_dsp;
  if (ogg_stream_init(&stream, 0x424F4633) != 0)
    goto clear_block;

  vorbis_analysis_headerout(&dsp, &comment, &header, &header_comment,
                            &header_code);
  ogg_stream_packetin(&stream, &header);
  ogg_stream_packetin(&stream, &header_comment);
  ogg_stream_packetin(&stream, &header_code);
  if (write_pages(f, &stream, 1) != 0)
    goto clear_stream;

  while (pos < frames) {
    int     count = (frames - pos > 1024) ? 1024 : (int)(frames - pos);
    float** buffer = vorbis_analysis_buffer(&dsp, count);
    int     i;

    for (i = 0; i < count; i++) {
      buffer[0][i] = pcm[(pos + i) * 2] / 32768.0f;
      buffer[1][i] = pcm[(pos + i) * 2 + 1] / 32768.0f;
    }
    vorbis_analysis_wrote(&dsp, count);
    pos += count;

    while (vorbis_analysis_blockout(&dsp, &block) == 1) {
      vorbis_analysis(&block, NULL);
      vorbis_bitrate_addblock(&block);
      while (vorbis_bitrate_flushpacket(&dsp, &packet)) {
        ogg_stream_packetin(&stream, &packet);
        if (write_pages(f, &stream, 0) != 0)
          goto clear_stream;
      }
    }
  }

  vorbis_analysis_wrote(&dsp, 0);
  while (vorbis_analysis_blockout(&dsp, &block) == 1) {
    vorbis_analysis(&block, NULL);
    vorbis_bitrate_addblock(&block);
    while (vorbis_bitrate_flushpacket(&dsp, &packet)) {
      ogg_stream_packetin(&stream, &packet);
      if (write_pages(f, &stream, 0) != 0)
        goto clear_stream;
    }
  }
  if (write_pages(f, &stream, 1) == 0)
    rc = 0;

clear_stream:
  ogg_stream_clear(&stream);
clear_block:
  vorbis_block_clear(&block);
clear_dsp:
  vorbis_dsp_clear(&dsp);
clear_comment:
  vorbis_comment_clear(&comment);
  vorbis_info_clear(&info);
  fclose(f);
  return rc;

clear_info:
  vorbis_info_clear(&info);
  fclose(f);
  return -1;
}

#else

int ogg_write_stereo(const char* path, const int16_t* pcm, int64_t frames,
                     int rate) {
  (void)path;
  (void)pcm;
  (void)frames;
  (void)rate;
  return -1;
}

#endif
