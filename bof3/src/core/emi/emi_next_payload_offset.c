#include "internal.h"

/* does: advances one EMI payload offset by sector-aligned size. */
u32 emi_next_payload_offset(u32 current_offset, u32 current_size) {
  u32 sector_count = (current_size + (EMI_SECTOR_SIZE - 1u)) >> 11;
  return current_offset + sector_count * EMI_SECTOR_SIZE;
}
