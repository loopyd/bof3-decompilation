#include "bof3/ui/shop00_internal.h"

/* The swap call stays inside each kind arm even though the two arms emit one
 * joined call site: writing the call once after a selected base pointer
 * measured a different register for that base and a displaced load of the
 * saved id, while the per-arm call cross-jumps into the original's single
 * `jal swapBytesC` with the base in $a1. */
/* @source 0x801DF478
 * @behavior exchanges the ids of the two shop selection cursors: the current
 *           cursor (kind byte D_80148658, slot byte D_80148659) and the saved
 *           cursor (kind byte D_8014865A, slot byte D_8014865D). When both
 *           kinds agree it swaps the two slots of the id list that kind
 *           selects (kind 0 = the visible list D_80144F5A, nonzero = the
 *           off-screen list D_801E6244) through swapBytesC and reports 0xFF.
 *           When they differ it reads the current id out of D_80144F5A with
 *           the slot of the list its kind selects and the saved id out of
 *           D_801E6244 with the other slot, then returns that id when its
 *           field record carries the bit-1 flag of
 *           D_8014496F[164 * D_80181B10[id]]. Otherwise it plays the main-exe
 *           cue 0x103, overwrites every D_80144F5A and D_80144F5D slot holding
 *           the current id with the saved id, overwrites every D_801E6244 slot
 *           holding the saved id with the current id, and reports 0xFF. The
 *           0xFF result tells the caller func_801DE4C8 that there is no new id
 *           to publish; every other result is stored into D_801E6254. Both
 *           slot loops count bytes (u8 counter, bound from the mode scanner
 *           func_801BDB7C(0) and from the count byte D_801E6250).
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 swapSelectionIds(void) {
  u8 src;
  u8 dst;
  s32 offset;
  u8 slot;

  if (D_80148658 == D_8014865A) {
    if (D_80148658 == 0) {
      swapBytesC(D_80144F5A + D_80148659, D_80144F5A + (s8)D_8014865D);
    } else {
      swapBytesC(D_801E6244 + D_80148659, D_801E6244 + (s8)D_8014865D);
    }
    return 0xFF;
  }
  if (D_80148658 == 0) {
    src = D_80144F5A[D_80148659];
    dst = D_801E6244[(s8)D_8014865D];
  } else {
    src = D_80144F5A[(s8)D_8014865D];
    dst = D_801E6244[D_80148659];
  }
  offset = D_80181B10[src] * 164;
  if ((D_8014496F[offset] & 2) != 0) {
    return src;
  }
  offset = D_80181B10[dst] * 164;
  if ((D_8014496F[offset] & 2) != 0) {
    return dst;
  }
  func_8015DF18(0x103);
  for (slot = 0; slot < func_801BDB7C(0); slot++) {
    if (D_80144F5A[slot] == src) {
      D_80144F5A[slot] = dst;
    }
    if (D_80144F5D[slot] == src) {
      D_80144F5D[slot] = dst;
    }
  }
  for (slot = 0; slot < D_801E6250; slot++) {
    if (D_801E6244[slot] == dst) {
      D_801E6244[slot] = src;
    }
  }
  return 0xFF;
}
