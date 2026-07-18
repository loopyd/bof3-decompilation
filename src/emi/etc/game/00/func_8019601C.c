#include "internal.h"

/* @behavior finds the first unused record slot in the entry table at
 * D_80143FC8 by scanning for a zero flags_00 byte; returns the slot
 * index (0‑19) or 0xFF when every slot is occupied.
 * The mode parameter is unused in this revision — the search always
 * starts from index 0.
 * @source 0x8019601C
 */
u8 func_8019601C(u8 mode) {
  u8 i;

  for (i = 0; i < 20; i++) {
    if (D_80143FC8[i].flags_00 == 0) {
      return i;
    }
  }
  return 0xFF;
}
