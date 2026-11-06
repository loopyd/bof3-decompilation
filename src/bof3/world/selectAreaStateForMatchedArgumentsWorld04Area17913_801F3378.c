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
 * Shape: the 108-byte range is byte-identical to areas 176, 177, 178 and
 *   180-185 at 0x801F3378 (payload span 0x778..0x7E4, one payload hash per
 *   twin). Area177/13 carries an exact source for the same bytes and this body
 *   copies its statement shape; the body holds no address-specific immediate,
 *   so this target's literals and symbol bindings come from the original
 *   bytes. The decisive part is the third guard written as a positive block
 *   whose body holds the store select and its own `return 1`, with `return 0`
 *   after the block: that keeps the at-least-8 arm inline and puts the below-8
 *   arm out of line behind `j`, leaving the branch delay slot empty. An early
 *   `return 0` guard followed by a function-level select instead merges both
 *   arms into one store block, fills that delay slot and drops the `j`, which
 *   measured 8 bytes short at 74.07%.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 selectAreaStateForMatchedArgumentsWorld04Area17913_801F3378(u16 arg0,
                                                                u16 arg1) {
  if (arg0 != 0x3F) {
    return 0;
  }
  if (arg1 != 0xE) {
    return 0;
  }
  if (D_80145E98 == 1) {
    if (D_80146870 >= 8) {
      D_801448EB = 0x1A;
    } else {
      D_801448EB = 6;
    }
    return 1;
  }
  return 0;
}
