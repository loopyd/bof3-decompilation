#ifndef GPU_UPLOAD_H
#define GPU_UPLOAD_H

#include "base/types.h"

/* Type-3 load_arg bitfield (graphics.md) */
#define GFX_LOAD_ARG_BASE_X(arg)     (((u32)(arg) >> 24) & 0xFFu)
#define GFX_LOAD_ARG_BASE_Y(arg)     (((u32)(arg) >> 16) & 0xFFu)
#define GFX_LOAD_ARG_CHUNKS_ROW(arg) (((u32)(arg) >> 8) & 0x3Fu)

enum {
  GFX_CHUNK_VRAM_WORDS = 32,
  GFX_CHUNK_W_4BPP = 128,
  GFX_CHUNK_W_8BPP = 64,
  GFX_CHUNK_W_16BPP = 32,
  GFX_CHUNK_H = 32,
};

#endif
