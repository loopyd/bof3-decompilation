#include "bof3/bof3.h"

extern u8 D_80145E98; /* @source 0x80145E98 @kind data */
extern s8 D_80146870; /* @source 0x80146870 @kind data */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

/* @source 0x801F3378
 * @behavior Selects the shared state-entry byte 0x801448EB from the signed
 *   scenario-state byte 0x80146870 once the overlay request matches: the
 *   function returns 0 unless the first two zero-extended u16 arguments read
 *   0x3F and 0xE and the leading byte of the main-RAM game-mode record
 *   0x80145E98 reads 1. On a match it stores 0x1A when the scenario-state byte
 *   is at least 8 and 6 when it is below 8, then returns 1.
 *
 * Name evidence: 0x801448EB is the state selector that game00 dispatches the
 *   handler table 0x801C84BC through (dispatchIndexedStateHandler reads it as
 *   an s8 table index) and that the world overlays set as their state entry;
 *   0x80146870 is the first byte of the etc/game/00 scenario-state record
 *   (scenarioState, declared s8 at the original `lb` width in
 *   include/bof3/ui/shop00_internal.h); 0x80145E98 is the leading byte of the
 *   main-RAM game-mode record table, read here with `lbu`. The two arguments
 *   carry no evidenced role, so the name keeps them structural.
 *
 * Shape: the 108-byte range is byte-identical to areas 176, 177, 178, 180,
 *   181, 182, 183, 184 and 185 at 0x801F3378 (payload span 0x778..0x7E4, one
 *   payload `cmp` per twin), and area177/13 already carries an exact source for
 *   it. The body holds no address-specific immediate, so that sibling's
 *   statement order, operand forms and declaration style are copied and this
 *   target's literals and symbol bindings are taken from the original bytes.
 *   The store sits in each arm because the original's single join `sb` is the
 *   cross-jumped tail of two arm stores; the matched arm order is the one the
 *   original falls into (0x1A) with the 6 arm as the jumped-to block.
 *
 * Residual: the first 17 instructions match. The tail is 8 bytes short: the
 *   original leaves the `bnez` delay slot empty (`nop`) and keeps an extra
 *   `j` to the join with the 0x1A constant in that jump's delay slot, while
 *   this profile folds the taken arm's constant into the branch delay slot and
 *   drops the jump. Fifteen clean-C spellings of the selection (ternary,
 *   if/else, per-arm stores, per-arm stores and returns, seeded value local,
 *   mirrored condition and arm order, direct-cell and local targets) all
 *   measured the same 100-byte 25-instruction body, and an `-O1` diagnostic of
 *   the same source measured the same tail, so the diff is not a select
 *   spelling. Every guard, load, conversion, store and return matches; the
 *   residual is branch-delay-slot filling plus a redundant join jump.
 * @status partial
 * @match 74.07
 * @residual Branch delay slot: original nop + extra j to the join, current
 *   fills the slot with the taken arm's constant and drops the jump.
 */
s32 selectAreaStateForMatchedArgumentsWorld04Area17913_801F3378(u16 arg0,
                                                                u16 arg1) {
  if (arg0 != 0x3F) {
    return 0;
  }
  if (arg1 != 0xE) {
    return 0;
  }
  if (D_80145E98 != 1) {
    return 0;
  }
  if (D_80146870 >= 8) {
    D_801448EB = 0x1A;
  } else {
    D_801448EB = 6;
  }
  return 1;
}
