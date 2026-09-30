#include "bof3/battle/battle03_internal.h"

/* @source 0x801D6818
 * @behavior Scans the battle-local work records from the D_801462E5 cursor up to
 * the volatile D_801462F0 count, skipping every record func_801DB524 reports
 * unavailable; for the first record whose D_80145FB8 flag word carries bit 0 or
 * bit 1 it raises flag 0x2000 on that record's D_80145FB4 flag word, allocates
 * the queued slot-store entry for kind 0xB when the record's D_80145F09 byte is
 * 4 and for kind 0xC otherwise, clearing bit 0 of the three 0x801EC048 status
 * records first, then publishes the record pointer into the entry +0x74 pointer
 * and advances both the cursor and the battle state byte 0x801462E2; when the
 * scan exhausts it advances only the state byte.
 * @status partial
 * @match 69.29
 * @residual non-exact live audit: 88/127 instructions, 488 versus 508 bytes; first difference at +0x0000, the original materializes the 0x801462E5 cursor address in $a0 for the entry guard and copies it into $s0 for the loop body while this candidate allocates $s0 up front, and the two 0x140-scale base hoists land on opposite sides (&D_80145F09 here versus &D_80145E90 in the original); every remaining difference is an address-form/register-allocation choice with the same instruction vocabulary.
 */
/* Matching note: the scan cursor is reached through a local pointer because the
 * original keeps its 0x801462E5 address in a single base register for the whole
 * body and reaches the paired battle state byte 0x801462E2 as a -3 displacement
 * from that base; expressing either byte through its own symbol emits a separate
 * `lui`/`addiu`/load pair instead of the shared base. The advance increment is
 * shared by both skip arms (`goto advance`), because the original has exactly one
 * increment-and-test block reached from both skips; duplicating the increment per
 * skip arm measured 76/131 instead of 88/127. */
void queueEligibleWorkRecord(void) {
  u8* cursor;
  u32 flags;
  u32 slot;
  u8  index;

  cursor = &D_801462E5;
  if (*cursor < D_801462F0) {
    do {
      if (func_801DB524(*cursor) != 0u) {
        goto advance;
      }

      flags = D_80145FB8[*cursor].flags_00;
      if ((flags & 1u) == 0u) {
        if ((flags & 2u) == 0u) {
          goto advance;
        }
      }

      D_80145FB4[*cursor].flags_00 |= 0x2000u;
      if (D_80145F09[*cursor * 0x140u] == 4u) {
        slot = func_801E590C(0u, 0xBu) & 0xffu;
      } else {
        index = 0u;
        do {
          battleLocalStatusArray[index].flags_00 &= 0xFFFFFFFEu;
          index += 1u;
        } while (index < 3u);
        slot = func_801E590C(0u, 0xCu) & 0xffu;
      }

      D_801EC330[slot].ptr_74 = (u32)&D_80145E90[*cursor];
      *cursor += 1u;
      cursor[-3] += 1u;
      return;

    advance:
      *cursor += 1u;
    } while (*cursor < D_801462F0);
  }

  BATTLE_GLOBAL_BYTE_62E2 += 1u;
}
