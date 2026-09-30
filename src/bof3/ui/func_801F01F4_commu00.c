#include "bof3/ui/commu00_internal.h"
#include <stdlib.h>

/* @source 0x801F01F4
 * @behavior Removes one active record from the fairy slot walk: picks a
 *           starting slot from rand() masked to the sixty-slot range, re-bases
 *           values at or above slot 0x3C by subtracting 0x3C, then walks the
 *           stride-8 active-record table in wraparound order from that slot.
 *           The first active slot whose kind byte is not 9 is cleared and its
 *           slot index is appended to the removal queue at 0x80145E30 through
 *           the append counter at 0x80145E44; a kind-9 slot is only accepted
 *           once the walk has returned to its starting slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F01F4(void) {
  u32 pick;
  u8  start;
  u8  index;
  u8  found;

  found = 0;
  pick = rand() & 0x3F;
  index = pick;
  if (index >= 0x3C) {
    index = pick - 0x3C;
  }
  start = index;

  for (;;) {
    if (activeRecordBytes[index].active != 0) {
      if (activeRecordBytes[index].kind != 9 || found != 0) {
        activeRecordBytes[index].active = 0;
        D_80145E30[D_80145E44] = index;
        D_80145E44 = D_80145E44 + 1;
        return;
      }
    }

    index++;
    if (index >= 0x3C) {
      index = 0;
    }
    if (index == start) {
      found = 1;
    }
  }
}
