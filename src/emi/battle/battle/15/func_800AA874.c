#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @tests a specific bit in the bitmask array at D_80144F80.
 * Extracts bit index from arg0 and tests it.
 * @source 0x800AA874
 */
u32 func_800AA874(u32 arg0) {
  u32 idx;

  idx = arg0 & 0xFF;
  return ((D_80144F80[idx >> 5] >> (idx & 0x1F)) & 1);
}
