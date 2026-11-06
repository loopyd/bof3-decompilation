#include "bof3/bof3.h"

/* Leading byte of the main-RAM game-mode record, read here with a single `lbu`.
 * @source 0x80145E98 @kind unknown */
extern u8 D_80145E98;
/* Signed leading byte of the shared scenario-state record, taken with `lb` and
 * tested against 8.
 * @source 0x80146870 @kind unknown */
extern s8 D_80146870;
/* Shared state-entry selector that the front-end dispatcher indexes its handler
 * table with and that the world overlays publish their next entry into.
 * @source 0x801448EB @kind unknown */
extern u8 D_801448EB;

/* @source 0x801F3378
 * @behavior Overlay request gate that selects the shared area state byte
 *   0x801448EB: it returns 0 unless the two zero-extended halfword arguments
 *   read 0x3F and 0x0E (each truncated with `andi 0xFFFF`, so both are u16) and
 *   the leading byte of the main-RAM game-mode record 0x80145E98 reads 1. On a
 *   match it stores 6 while the signed scenario byte 0x80146870 is below 8 and
 *   0x1A otherwise, then returns 1. In the original the 0x1A `addiu` rides in
 *   the `j` delay slot, the below-8 arm is out of line and the `bnez` delay
 *   slot stays `nop`; every failing guard answers through the shared `jr $ra`
 *   with `addu $v0,$zero,$zero` in its branch delay slot. No calls and no
 *   frame.
 *
 * Name evidence: 0x801448EB is the state-entry selector the front-end
 *   dispatcher indexes its handler table with and that world overlays publish
 *   their next state into, 0x80146870 is the signed leading byte of the shared
 *   scenario-state record and 0x80145E98 the leading byte of the main-RAM
 *   game-mode record. The two arguments carry no evidenced role, so the name
 *   keeps them structural; it reuses the role name that the sibling sources of
 *   emi/world04/area176/13, area177/13, area179/13 and area181/13 give this
 *   same overlay function at 0x801F3378.
 *
 * Shape: the 108-byte span is byte-identical to the already-exact source of
 *   emi/world04/area176/13 (`bin/harness lift clone --from
 *   emi/world04/area176/13@0x801F3378` reports 108 identical bytes, and one
 *   payload `cmp` of span 0x778..0x7E4 matches areas 176, 177, 179, 180, 181,
 *   182, 183, 184 and 185 while area174 and world03/area121/14 differ). This
 *   body holds no address-specific immediate, so that sibling's statement
 *   order, operand forms and declaration style are copied and the literals and
 *   symbol bindings come from this target's bytes.
 *
 *   The third guard is the decisive shape. Written as a positive block whose
 *   body holds the store select and its own `return 1`, with `return 0` after
 *   the block, it keeps the at-least-8 arm inline and puts the below-8 arm out
 *   of line behind `j` with an unfilled branch delay slot, which is the
 *   original's 9-instruction tail. Seeded earlier from an early `return 0`
 *   third guard at function level, this file measured 20/27 at 100 bytes with
 *   the first difference at that tail: dbr inverts the branch to the join,
 *   fills its delay slot with the below-8 constant and drops the `j` and the
 *   `nop`.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 selectAreaStateForMatchedArgumentsWorld04Area17813_801F3378(u16 arg0,
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
