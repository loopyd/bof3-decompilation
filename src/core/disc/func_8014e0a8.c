#include "bof3/context.h"
#include "internal.h"

/* @behavior starts one disc read sized from a byte count and waits until the read
 * finishes or returns a terminal status.
 * @source 0x8014e0a8 FUN_8014e0a8
 */
s32 func_8014e0a8(s32 size, void* buffer, s32 sectors) {
  s32 sector_count;
  s32 status;

  sector_count = size + 0x7ff;
  if (sector_count < 0) {
    sector_count = size + 0xffe;
  }

  func_80178138(sector_count >> 11, buffer, sectors);

  while (1) {
    status = func_80178218(1, NULL);
    if (status <= 0) {
      return status;
    }

    func_80174700(0);
  }
}
