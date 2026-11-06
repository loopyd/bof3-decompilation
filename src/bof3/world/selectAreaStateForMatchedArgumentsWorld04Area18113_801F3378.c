#include "bof3/bof3.h"

extern u8 D_80145E98; /* @source 0x80145E98 @kind data */
extern s8 D_80146870; /* @source 0x80146870 @kind data */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

/* @source 0x801F3378
 * @behavior Selects the shared state-entry byte 0x801448EB from the signed
 *   scenario-state byte 0x80146870 once the overlay request matches: the
 *   function returns 0 unless the first two zero-extended u16 arguments read
 *   0x3F and 0xE and the leading byte of the main-RAM game-mode record
 *   0x80145E98 reads 1. On a match it stores 6 when the scenario-state byte is
 *   below 8 and 0x1A otherwise, then returns 1.
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
 * Shape: the 108-byte range is byte-identical to the world04 area overlays at
 *   this address (payload span 0x778..0x7E4 compared with areas 176 and 178
 *   through 185; `cmp` against emi/world04/area176/13.bin matches all 108
 *   bytes at the same load address), and the body holds no address-specific
 *   immediate. The third guard is the decisive shape: written as a positive
 *   block whose body holds the store select and its own `return 1`, with
 *   `return 0` after the block, it keeps the at-least-8 arm inline and puts
 *   the below-8 arm out of line behind `j` with an unfilled branch delay slot
 *   (`slti` / `bnez` / `nop` / `j` / `li 0x1A` / `li 6` / `sb`). An early
 *   `return 0` guard followed by the same select at function level let the
 *   owning profile relax the diamond into one inverted branch and built eight
 *   bytes short at 20/27.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 selectAreaStateForMatchedArgumentsWorld04Area18113_801F3378(u16 arg0,
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
