#include "bof3/ui/game00_internal.h"

/* @source 0x801BDB7C
 * @behavior scans 3 bytes in a mode-indexed table at D_80144F5A for 0xFF;
 *            returns index of first match (0-2), or 3 if none.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 findModeFreeSlot(u8 mode) {
  s32 index = 0;

  u32 sentinel = 0xFFu;

  unsigned int masked = mode & sentinel;
  u8* base = D_80144F5A;
  s32                   off = (s32)masked * 3;
  u8*                   ptr = base + off;

  while (index < 3) {
    if (*ptr == sentinel) {
      break;
    }
    index++;
    ptr++;
  }
  return index;
}
