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
 *   this address (payload span 0x778..0x7E4 compared across areas 176 through
 *   185; `lift clone` against emi/world04/area179/13@0x801F3378 reports the
 *   same 108 bytes), and the body holds no address-specific immediate. The
 *   comparison result is bound to the same local the arms overwrite, which is
 *   the measured best of fourteen probed clean-C select shapes: it keeps the
 *   original's `lb`/`slti`/`bnez` order and the single `sb`, but the owning
 *   profile's jump and delayed-branch passes still relax the select, so the
 *   original's out-of-line then-arm behind `j` with an unfilled branch delay
 *   (`bnez` / `nop` / `j` / `li 0x1A` / `li 6` / `sb`) is not reproduced and
 *   the body builds eight bytes short. Under
 *   -O2 -G0 -funsigned-char -msoft-float -gcoff the jump and delayed-branch
 *   passes delete that nop+j for every clean-C select shape measured, leaving
 *   the 5-instruction branch-around tail at 100 bytes, so the residual is the
 *   deleted delay slot and not a wrong operation or a wrong store value.
 * @status partial
 * @match 74.07
 * @residual Deleted nop+j: owning profile relaxes every clean-C select tail
 */
s32 selectAreaStateForMatchedArgumentsWorld04Area18113_801F3378(u16 arg0,
                                                                u16 arg1) {
  s32 state;

  if (arg0 != 0x3F) {
    return 0;
  }
  if (arg1 != 0xE) {
    return 0;
  }
  if (D_80145E98 != 1) {
    return 0;
  }
  state = D_80146870 < 8;
  if (state) {
    state = 6;
  } else {
    state = 0x1A;
  }
  D_801448EB = state;
  return 1;
}
