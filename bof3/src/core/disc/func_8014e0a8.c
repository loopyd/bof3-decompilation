#include "internal.h"

s32 CdRead(s32 sector_count, void* buffer,
           s32 sectors) __asm__("func_80178138");
s32 CdReadSync(s32 arg0, void* arg1) __asm__("func_80178218");
s32 VSync(s32 arg0) __asm__("func_80174700");

/* does: starts one disc read sized from a byte count and waits until the read
 * finishes or returns a terminal status.
 * @source: 0x8014e0a8 FUN_8014e0a8
 */
s32 func_8014e0a8(s32 size, void* buffer, s32 sectors) {
  s32 sector_count;
  s32 status;

  sector_count = size + 0x7ff;
  if (sector_count < 0) {
    sector_count = size + 0xffe;
  }

  CdRead(sector_count >> 11, buffer, sectors);

  while (1) {
    status = CdReadSync(1, NULL);
    if (status <= 0) {
      return status;
    }

    VSync(0);
  }
}
