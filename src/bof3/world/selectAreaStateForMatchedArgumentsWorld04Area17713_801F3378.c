#include "bof3/bof3.h"

extern u8 D_80145E98; /* @source 0x80145E98 @kind data */
extern s8 D_80146870; /* @source 0x80146870 @kind data */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

/* @source 0x801F3378
 * @behavior Selects the shared area state byte 0x801448EB from the signed
 *   scenario progress byte 0x80146870 once the overlay request matches: the
 *   function returns 0 unless the first two zero-extended u16 arguments read
 *   0x3F and 0xE and the leading byte of the main-RAM game-mode record
 *   0x80145E98 reads 1. On a match it stores 6 when the progress byte is below
 *   8 and 0x1A otherwise, then returns 1. No calls, no frame, no other stores.
 *
 * Name evidence: 0x801448EB is the state selector that other overlays dispatch
 *   through their handler tables (etc/game/00 reads it as an s8 table index,
 *   world01/area044/13 treats it as the shared area task state byte), and the
 *   0x80146870/0x80145E98 roles come from their declarations in
 *   include/bof3/world/area01613_internal.h and
 *   include/bof3/ui/shop00_internal.h. The two arguments carry no evidenced
 *   role, so the name keeps them structural.
 *
 * Shape: the 108-byte range is byte-identical across nine world04 area
 *   overlays (payload span 0x778..0x7E4 of areas 176, 178, 179, 180, 181, 182,
 *   183, 184 and 185 compared against this target), but every one of those
 *   rows is still an `asm` boundary, so there is no verified exact sibling
 *   source to clone or copy from and the body is seeded from this target's own
 *   bytes.
 *
 *   The two select arms are written as one store per arm so that the build
 *   keeps the original's `bnez` on the `slti` result and materialises both
 *   constants in `$v0`; a single conditional expression feeding the store
 *   instead builds `beqz` with the else constant in the branch delay slot.
 * @status partial
 * @match 74.07
 * @residual The original keeps `nop` in the branch delay slot and reaches the
 *   below-8 arm through an 8-byte `j`, so this build is 8 bytes short and ends
 *   at 20/27 instructions. The jump optimizer merges the two arms in both
 *   `jump` and `jump2` (verified in cc1 dumps of this source): it redirects the
 *   branch to the shared store and fills the branch delay slot with the below-8
 *   constant. Every probed clean-C spelling of the select reached the same
 *   merged body: conditional expression feeding the cell, conditional
 *   expression into a u8/s32 local, each arm assigning one local, one local
 *   reused for the comparison and the result, explicit `goto` joins, mirrored
 *   `>=` conditions, a `switch` on the comparison, and a cast at the store.
 */
s32 selectAreaStateForMatchedArgumentsWorld04Area17713_801F3378(u16 arg0, u16 arg1) {
  if (arg0 != 0x3F) {
    return 0;
  }
  if (arg1 != 0xE) {
    return 0;
  }
  if (D_80145E98 != 1) {
    return 0;
  }
  if (D_80146870 < 8) {
    D_801448EB = 6;
  } else {
    D_801448EB = 0x1A;
  }
  return 1;
}
