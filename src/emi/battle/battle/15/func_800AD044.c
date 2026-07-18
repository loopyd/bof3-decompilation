#include "internal.h"

/* @tests a specific bit in the bitmask array at D_80144F60.
 * Uses arg0 & 0xFFFF for index calculation.
 * @source 0x800AD044
 */
u32 func_800AD044(u32 arg0) {
  u32 idx;

  idx = arg0 & 0xFFFF;
  return ((D_80144F60[idx >> 5] >> (idx & 0x1F)) & 1);
}
