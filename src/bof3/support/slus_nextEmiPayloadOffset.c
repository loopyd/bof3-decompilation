#include "bof3/core/slus_internal.h"

/* @behavior advances one EMI payload offset by sector-aligned size. */
u32 nextEmiPayloadOffset(u32 current_offset, u32 current_size) {
  u32 sector_count = (current_size + (EMI_SECTOR_SIZE - 1u)) >> 11;
  return current_offset + sector_count * EMI_SECTOR_SIZE;
}
