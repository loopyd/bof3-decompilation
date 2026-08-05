#include "internal.h"

/* @tests a specific bit in the bitmask array at D_80144F80.
 * Extracts bit index from arg0 and tests it.
 * @source 0x800AA874
 */
u32 func_800AA874(u32 arg0) {
  arg0 &= 0xFF;
  return (*(u32 *)((u8 *)D_80144F80 + ((arg0 >> 5) * 4)) & (1 << (arg0 & 0x1F))) != 0;
}
