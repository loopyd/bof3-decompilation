#include "bof3/bof3.h"

/* Battle record pointer published at 0x8014638C in main RAM, outside this
 * overlay image. The exact lifts of this target reach the same cell: the record
 * is aimed at, submitted and range-tested through it. This body keeps the cell
 * address in a local because its two loads share one lui/addiu base in a
 * callee-saved register across the first call; writing both accesses on the
 * named symbol instead folds the address into each load and loses that base.
 * Each load still re-reads the cell, because the helper may republish it. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Scratchpad work-object pointer cell, spelled with the named cell the exact
 * lifts of this target already use. A named cell symbol lets the assembler fold
 * the constant address into the load - lui $v1,0x1F80 / lw $v1,0x44($v1) - and
 * the original repeats the `nop` after that load, so the single use below stays
 * one load plus one store. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Record helper of the main executable at 0x800AF2B4, not lifted, so its effect
 * stays unestablished. The original passes a record base and a constant; this
 * step passes the battle record and 0x60, while the sibling step at 0x801EF57C
 * passes 3 and the boss callers 0x50. It returns nothing. */
extern void func_800AF2B4(u8 *record, u32 value);

/* Record range test of the main executable at 0x800AF66C. The battle lift
 * testWorkRangeAgainstRecord owns the body: it advances the scratch work
 * object's 0x34 and 0x38 axes by half the passed value, reduces each by the
 * record's matching origin and returns 1 only when the value reaches both
 * bounds. This step passes 0x20000 and branches on the result word. */
extern u32 func_800AF66C(u8 *record, u32 value);

/* @source 0x801EFA70
 * @behavior Work byte 2 step 1 handler of this overlay, reached from the
 *           three-entry work byte 2 handler table at 0x801EEC3C (0x801EF928,
 *           0x801EFA70, 0x801EFAD4) that the dispatcher 0x801EF894 selects by the
 *           work object's unsigned byte 2 with no argument setup and no bounds
 *           test, so no jal in this payload names it. It first passes the battle
 *           record published at 0x8014638C to the main-executable record helper
 *           func_800AF2B4 with the constant 0x60, then re-reads the cell and
 *           passes the same record to the range test func_800AF66C with the
 *           constant 0x20000. Only when that test reports a hit does it re-read
 *           the scratchpad work-object pointer cell at 0x1F800044 and advance the
 *           work object's unsigned byte 2, the step byte this handler was
 *           selected by, by one. It takes no arguments, returns nothing and
 *           writes no other state. The cell-address local is hoisted into $s0
 *           ahead of the $ra store, the 0x18-byte frame only keeps $s0 at 0x10
 *           and $ra at 0x14, and the slots after each call, after the range
 *           branch, after the cell load and after the byte load stay nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyHelper60ToBattleRecordThenAdvanceWorkByte2WhenWorkWithin20000OfRecordBmagicMagic02003_801EFA70(
    void) {
  u8 **recordCell;
  u8 *work;

  recordCell = &D_8014638C;
  func_800AF2B4(*recordCell, 0x60);
  if (func_800AF66C(*recordCell, 0x20000)) {
    work = D_1F800044;
    work[2]++;
  }
}
