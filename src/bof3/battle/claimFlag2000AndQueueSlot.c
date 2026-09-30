#include "bof3/battle/battle03_internal.h"

/* @source 0x801E12DC
 * @behavior Scans the D_801462F0 active local work records for flag 0x2000 and returns as soon as one carries it; otherwise sets that flag on the current work record published at 0x80146250, allocates the queued-slot entry through func_801E590C for kind 0xB when work byte +0x79 is 4 and 0xC otherwise, publishes the scratch work pointer into that entry and increments scratch byte +0x04.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E12DC(void) {
  Battle03LocalWork* work;
  Battle03LocalWork* scratch;
  u32                slot;
  u8                 count;
  u8                 index;

  /* the original caches this count byte once for the whole scan; the header's
   * volatile view would force a per-iteration reload, so read it through a
   * plain u8 view. */
  count = *(u8*)&D_801462F0;
  index = 0;
  if (count != 0) {
    do {
      if ((D_80145FB4[index].flags_00 & 0x2000u) != 0u) {
        return;
      }
      index += 1;
    } while (index < count);
  }
  work = D_80146250;
  work->unk_124 |= 0x2000u;
  if (work->unk_79 == 4) {
    slot = func_801E590C(0u, 0xBu) & 0xffu;
  } else {
    slot = func_801E590C(0u, 0xCu) & 0xffu;
  }
  scratch = D_1F800044;
  D_801EC330[slot].ptr_74 = (u32)scratch;
  scratch->unk_04++;
}
