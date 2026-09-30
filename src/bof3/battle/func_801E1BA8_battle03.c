#include "bof3/battle/battle03_internal.h"

/* @source 0x801E1BA8
 * @behavior Allocates the queued slot-store entry for mode byte 5 through
 * func_801E590C, then publishes the scratch record position halfwords +0x2e and
 * +0x30 into that entry as the scratch half minus the signed byte pair selected
 * by local work byte +0x79 in the 2-byte-stride table at D_80181D98, and
 * increments the scratch record byte +0x02.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E1BA8(void) {
  Battle03LocalWork* work;
  u8*                scratch;
  Battle03SlotStore* record;
  u32                index;

  index = func_801E590C(0u, 5u) & 0xffu;
  record = &D_801EC330[index];
  work = D_80146250;
  scratch = (u8*)D_1F800044;
  /* the deltas are loaded unsigned (lbu) and sign-extended for the subtraction */
  record->unk_2e = *(u16*)(scratch + 0x2e) - (s8)D_80181D98[work->unk_79 * 2];
  record->unk_30 = *(u16*)(scratch + 0x30) - (s8)D_80181D99[work->unk_79 * 2];
  scratch[2]++;
}
