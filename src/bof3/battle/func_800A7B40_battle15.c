#include "bof3/battle/battle15_internal.h"

/* @source 0x800A7B40
 * @behavior Appends the non-zero bytes of the three-byte group list at
 * 0x800B4EB0 selected by group into the 0x70-offset slot array of the current
 * battler record (0x80145F04 + D_80146374 * 0x140), skipping group bytes that
 * are already present in the first slot slots; returns the resulting slot
 * count, or 9 when all nine slots are occupied.
 * @status partial
 * @match 77.78
 * @residual first mismatch +0x0008: the preheader materialises the list base
 * (lui/addiu) after the index scaling instead of before it, the compiler hoists
 * the `slot == 9` compare constant into a register live across the loop (t2
 * instead of the original's guard zero; the guard zero moves to t3), and the
 * full-check branch is inverted with an extra `j`, so the 9 constant is not
 * reused as the return value as in the original. Same instruction multiset as
 * the original minus that `j`, 212->216 bytes; 42/54 instructions, no byte
 * match. Clean-C alternatives measured: single-shared-return, break/goto exit,
 * if/else inversion, `entry` local, index-last spelling, `i = 0` hoist, post
 * increment, result variable (see the lane report; none exceeded 42/54).
 */
u32 func_800A7B40(u8 group, u8 slot)
{
  u8 i;
  u8 j;
  u8* row;
  u8* slots;

  i = 0;
  row = &D_800B4EB0[(u32)group * 3u];
  slots = &D_80145F04[(u32)D_80146374 * 0x140u];

  for (; i < 3u; i++) {
    if (row[i] == 0u) {
      return slot;
    }
    for (j = 0; j < slot; j++) {
      if (slots[0x70 + j] == row[i]) {
        goto next;
      }
    }
    if (slot == 9u) {
      return 9u;
    }
    slots[0x70 + slot] = row[i];
    slot++;
  next:
    ;
  }
  return slot;
}
