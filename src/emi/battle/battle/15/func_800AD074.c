#include "internal.h"

/* @sets a specific bit in the bitmask array at D_80144F60 based on arg0.
 * Uses arg0 & 0xFFFF for index calculation.
 * @source 0x800AD074
 */
void func_800AD074(u32 arg0) {
  u32 idx;

  idx = arg0 & 0xFFFF;
  D_80144F60[idx >> 5] |= (1U << (idx & 0x1F));
}
