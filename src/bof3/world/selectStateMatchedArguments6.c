#include "bof3/bof3.h"

extern u8 D_80145E98; /* @source 0x80145E98 @kind data */
extern s8 D_80146870; /* @source 0x80146870 @kind data */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

/* @source 0x801F3378
 * @behavior Returns 0 unless the two zero-extended u16 arguments read 0x3F and
 *   0x0E and the leading byte of the main-RAM game-mode record 0x80145E98
 *   reads 1; on that match it stores 6 into the shared state-entry byte
 *   0x801448EB while the signed scenario-state byte 0x80146870 is below 8 and
 *   0x1A once it reads at least 8, then returns 1. Leaf body: no calls, no
 *   frame and no stack use, only two byte reads, one byte store and the
 *   register compares.
 *
 * Name evidence: 0x801448EB is the state selector the front-end dispatcher
 *   indexes its handler table 0x801C84BC with and that the world overlays
 *   publish their next state into, 0x80146870 is the signed leading byte of
 *   the shared scenario-state record (declared scenarioState in
 *   include/bof3/ui/shop00_internal.h) and 0x80145E98 the leading byte of the
 *   main-RAM game-mode record table (declared in
 *   include/bof3/ui/game00_internal.h). The two arguments carry no evidenced
 *   role, so the name keeps them structural; it reuses the role name the
 *   sibling overlay sources of emi/world04/area176/13, area177/13, area179/13
 *   and area181/13 give this same function at 0x801F3378.
 *
 * Shape: the 108-byte span is byte-identical to the already-exact source of
 *   emi/world04/area181/13 (`bin/harness lift clone --from
 *   src/bof3/world/selectAreaStateForMatchedArgumentsWorld04Area18113_801F3378.c`
 *   reported 108 identical bytes with no identifier rewrites), and one payload
 *   comparison of span 0x778..0x7E4 matches areas 176 through 185 at this
 *   target's own load address. The body holds no address-specific immediate,
 *   so that sibling's statement order, operand forms and declaration style are
 *   copied while this target's literals and symbol bindings come from its own
 *   bytes.
 *
 *   The decisive shape is the third guard written as a positive block whose
 *   body holds the store select and its own `return 1`, with `return 0` after
 *   the block: that keeps the at-least-8 arm inline and puts the below-8 arm
 *   out of line behind `j` with an unfilled branch delay slot, which is the
 *   original's nine-instruction tail. Seeded instead as an early `return 0`
 *   third guard at function level, the emi/world04/area178/13 source of this
 *   span measured 20 of 27 instructions at 100 bytes before its exact seed
 *   replaced it, with the first difference at that tail.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 selectStateMatchedArguments(u16 arg0,
                                                                u16 arg1) {
  if (arg0 != 0x3F) {
    return 0;
  }
  if (arg1 != 0x0E) {
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
