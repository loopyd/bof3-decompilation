#ifndef GPU_PALETTE_H
#define GPU_PALETTE_H

#include "base/types.h"
#include "memory/access.h"

#define g_PaletteDst PSX_PTR(volatile u16, 0x80037800u)

enum {
  PAL_ROW_4BPP = 0x20,
  PAL_ROW_8BPP = 0x200,
};

#define PSX_COLOR_R(c) ((u16)(c) & 0x1Fu)
#define PSX_COLOR_G(c) (((u16)(c) >> 5) & 0x1Fu)
#define PSX_COLOR_B(c) (((u16)(c) >> 10) & 0x1Fu)
#define PSX_COLOR_STP(c) (((u16)(c) >> 15) & 1u)
#define PSX_PACK_COLOR(r, g, b, stp) \
  ((u16)((r) | ((g) << 5) | ((b) << 10) | ((u16)(stp) << 15)))

#endif
