#include "bof3/battle/battle03_internal.h"

/* @source 0x801DD978
 * @behavior Scans the enemy work slots `3..10` that func_801DB524 reports
 * available and remembers the largest record halfword +0x88 seen; it then picks,
 * among the available slots whose halfword +0x88 still equals that maximum, the
 * first one whose halfword +0x94 is the smallest and publishes that slot index
 * in the battle byte 0x80146384.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DD978(void) {
  u8  best;
  u8  selected;
  u16 lowest;
  u16 value;
  u8  slot;

  best = 0u;

  slot = 3u;
  do {
    if (func_801DB524(slot) == 0u) {
      value = D_801EB630[slot - 3u].unk_88;
      if (best < value) {
        best = value;
      }
    }
    slot += 1u;
  } while (slot < 0x0Bu);

  lowest = 0xFFFFu;
  selected = 0u;
  slot = 3u;
  do {
    if (func_801DB524(slot) == 0u) {
      if (best == D_801EB630[slot - 3u].unk_88) {
        value = D_801EB630[slot - 3u].unk_94;
        if (value <= lowest) {
          lowest = value;
          selected = slot;
        }
      }
    }
    slot += 1u;
  } while (slot < 0x0Bu);

  D_80146384 = selected;
}
