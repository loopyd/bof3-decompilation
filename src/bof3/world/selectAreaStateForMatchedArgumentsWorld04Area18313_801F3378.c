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
 *   the shared scenario-state record and 0x80145E98 the leading byte of the
 *   main-RAM game-mode record. The two arguments carry no evidenced role, so
 *   the name keeps them structural; it reuses the role name the nine
 *   byte-identical overlay sources of this same address give it, which is the
 *   selectAreaStateForMatchedArguments family of emi/world04/area176/13
 *   through area182/13.
 *
 * Shape: the 108-byte span at 0x778..0x7E4 is byte-identical to the payload
 *   spans of emi/world04/area176/13, area177/13, area178/13, area179/13,
 *   area180/13, area181/13, area182/13 and area185/13 at this target's own
 *   load address 0x801F2C00 (one sha256 of the 108 bytes per payload, all
 *   f7117926a28888f193554b10bc8d981b38007893529ed29c39e2bd030f8442ee), and
 *   `bin/harness lift clone --from
 *   src/bof3/world/selectAreaStateForMatchedArgumentsWorld04Area18213_801F3378.c`
 *   reported 108 identical bytes with no identifier rewrites. That exact
 *   sibling supplied this statement order, operand forms and declaration
 *   style, and this target's literals, widths and symbol bindings were read
 *   back from its own bytes.
 *
 *   The decisive shape is the third guard written as a positive block whose
 *   body holds the store select and its own `return 1`, with `return 0` after
 *   the block: that keeps the at-least-8 arm inline and puts the below-8 arm
 *   out of line behind `j` with an unfilled branch delay slot, which is the
 *   original's nine-instruction tail. The family recorded 20 of 27
 *   instructions at 100 bytes when this guard was seeded as an early return at
 *   function level, with the first difference at that tail.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 selectAreaStateForMatchedArgumentsWorld04Area18313_801F3378(u16 arg0,
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
