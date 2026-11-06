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
 * Shape: the 108-byte span is byte-identical to those sibling sources
 *   (`bin/harness lift clone --from emi/world04/area177/13@0x801F3378` reports
 *   108 identical bytes, and one payload `cmp` of span 0x778..0x7E4 matches
 *   areas 176, 177, 179, 180, 181, 182, 183, 184 and 185 while area174 and
 *   world03/area121/14 differ). This body holds no address-specific immediate,
 *   so the sibling statement order, operand forms and declaration style are
 *   copied and the literals and symbol bindings come from this target's bytes.
 * @status partial
 * @match 74.07
 * @residual The first 17 instructions match; the range builds 8 bytes short
 *   (20/27 instructions, first difference at +0x0008, the guard branch target).
 *   This per-arm store form is the shape whose post-jump2 RTL already equals
 *   the original: `slti / beq-v0-0 -> else / store 6 / goto join / else:
 *   store 26 / join:` cross-jumps into `slti / branch / single arm / j join /
 *   li 0x1A / join: sb` (probe.c.jump2), so the source shape and the shared
 *   store are settled. dbr then inverts that branch to the join and fills its
 *   delay slot with the below-8 constant (probe.c.dbr, `insn 118` sequence),
 *   which removes both the `j` and the `nop` the original keeps, and that is
 *   the whole residual. Sixteen clean-C select spellings (ternary, if/else and
 *   seeded-local forms, s8/u8/s32, both condition polarities, store-per-arm and
 *   store-at-join) measure 15/27 to 20/27 at the same 100 bytes with the same
 *   first difference, so no arm order, local type or store position closes it.
 *   Deferred evidence: a build-profile question (the delay-slot fill decision
 *   itself), which is parent-owned and not probed here.
 */
s32 selectAreaStateForMatchedArgumentsWorld04Area17813_801F3378(u16 arg0,
                                                                u16 arg1) {
  if (arg0 != 0x3F) {
    return 0;
  }
  if (arg1 != 0x0E) {
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
