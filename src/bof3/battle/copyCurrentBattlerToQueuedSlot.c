#include "bof3/battle/battle03_internal.h"

/* @behavior allocates one slot-store entry and copies the current scratch object's
 * pointer plus words `0x34/0x38/0x3c` into that entry.
 * @source 0x801DDAF0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void copyCurrentBattlerToQueuedSlot(void) {
  Battle03LocalWork* scratch;
  Battle03LocalWork* work;
  u32 index;

  index = func_801E590C(0u, 6u) & 0xffu;
  scratch = *(Battle03LocalWork* volatile*)&D_1F800044;
  D_801EC330[index].flag_09 = 0u;
  work = *(Battle03LocalWork* volatile*)&D_1F800044;
  D_801EC330[index].ptr_74 = (u32)scratch;
  D_801EC330[index].unk_34 = (u32)work->unk_34;
  D_801EC330[index].unk_38 = (u32)work->unk_38;
  D_801EC330[index].unk_3c = *(u32*)((u8*)work + 0x3cu);
}
