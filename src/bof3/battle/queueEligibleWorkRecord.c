#include "bof3/battle/battle03_internal.h"

/* @source 0x801D6818
 * @behavior Scans the battle-local work records from the D_801462E5 cursor up to
 * the volatile localBattlerCount, skipping every record isBattlerInactiveOrFlag4000 reports
 * unavailable; for the first record whose D_80145FB8 flag word carries bit 0 or
 * bit 1 it raises flag 0x2000 on that record's D_80145FB4 flag word, allocates
 * the queued slot-store entry for kind 0xB when the record's D_80145F09 byte is
 * 4 and for kind 0xC otherwise, clearing bit 0 of the three 0x801EC048 status
 * records first, then publishes the record pointer into the entry +0x74 pointer
 * and advances both the cursor and the battle state byte 0x801462E2; when the
 * scan exhausts it advances only the state byte.
 * @status partial
 * @match 85.83
 * @residual 109/127 instructions, 508 original bytes versus 496. The first live
 * difference is +0x0004: the volatile D_801462F0 count load is scheduled ahead of
 * the guard address instead of between the frame stores and the comparison. The
 * remaining kind-byte address differs for work_base + 0x79, along with
 * the longer suffix gcc shares between the two kind arms (the slot-entry scale
 * and the record-pointer add). The frame is 32 bytes with the saved set
 * {ra,s2,s1,s0} as in the original.
 */
/* Matching note: the scan cursor keeps the addressed 0x801462E5 byte in one base
 * register for the whole body, which also reaches the paired state byte
 * 0x801462E2 as a -3 displacement; the advance increment is shared by both skip
 * arms because the original has exactly one increment-and-test block. The kind
 * byte is the +0x79 member of the same 0x140-byte record view, so one record
 * base serves the scan and the publish. */
void queueEligibleWorkRecord(void) {
  u8* cursor;
  u8* start;
  u8* work_base;
  u32 flags;
  u32 slot;
  u8  index;
  u8  current;

  start = &D_801462E5;
  if (*start < localBattlerCount) {
    cursor = start;
    work_base = (u8*)D_80145E90;
    do {
      if (isBattlerInactiveOrFlag4000(*cursor) != 0u) {
        goto advance;
      }
      flags = D_80145FB8[*cursor].flags_00;
      if ((flags & 1u) == 0u) {
        if ((flags & 2u) == 0u) {
          goto advance;
        }
      }
      D_80145FB4[*cursor].flags_00 |= 0x2000u;
      if (((u8*)work_base)[*cursor * 0x140u + 0x79u] == 4u) {
        slot = func_801E590C(0u, 0xBu) & 0xffu;
        current = *cursor;
      } else {
        index = 0u;
        do {
          battleLocalStatusArray[index].flags_00 &= 0xFFFFFFFEu;
          index += 1u;
        } while (index < 3u);
        slot = func_801E590C(0u, 0xCu) & 0xffu;
        current = D_801462E5;
      }
      D_801EC330[slot].ptr_74 = (u32)(work_base + current * 0x140u);
      *cursor += 1u;
      cursor[-3] += 1u;
      return;
    advance:
      *cursor += 1u;
    } while (*cursor < localBattlerCount);
  }
  {
    volatile u8* state;

    state = &BATTLE_GLOBAL_BYTE_62E2;
    *state += 1u;
  }
}
