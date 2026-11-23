#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The compiled original re-materializes the cell address for its one
 * access (lui $v1,0x1F80 / lw $v1,0x44($v1)), so the cell is reached through
 * this target-map symbol rather than through a constant address. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle global halfword at 0x801462E8, outside this overlay image. It
 * lives in the main executable, so it keeps its address-qualified name and
 * Splat binds it through this target generated undefined_syms_auto.txt row, so
 * no target-map row is added here. This body reads the halfword once and only
 * tests bit 0x0004; the bmagic lifts that raise that bit declare the same
 * volatile halfword. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* Shared main-executable helpers at 0x8014D978 and 0x8014D4E0, outside this
 * overlay image. This body calls both with no arguments and discards their
 * results, and Splat binds the two addresses through this target generated
 * undefined_funcs_auto.txt rows. Nothing in this payload establishes a
 * narrower role, so the names stay address-anchored. */
extern void func_8014D978(void); /* @source 0x8014D978 @kind function */
extern void func_8014D4E0(void); /* @source 0x8014D4E0 @kind function */

/* @source 0x801F007C
 * @behavior Invokes the shared main-executable helpers at 0x8014D978 and
 *           0x8014D4E0 in that order with no arguments, discarding their
 *           results, then reads the shared battle global halfword at
 *           0x801462E8 once and tests bit 0x0004. Only when that bit is set
 *           does it re-materialize the scratchpad pointer cell at 0x1F800044,
 *           reload the work-object pointer and increment the object byte at
 *           offset 0x2 modulo 256 (lbu / addiu 1 / sb), which advances the
 *           byte-2 dispatcher to the next handler step. It keeps only $ra at
 *           0x10 of its 0x18-byte frame across the two calls, takes no
 *           arguments, returns nothing through the jr $ra at 0x801F00D0 and
 *           reads or writes no other state. No in-image jal names it: it is
 *           entry index 1 of the three-word work-byte-2 handler table based at
 *           0x801EEC10 (0x801F001C, 0x801F007C, 0x801F00D8) that the dispatcher
 *           at 0x801EFFBC indexes with work-object byte 2, so the table word
 *           proves the function-entry boundary at 0x801F007C. The roles of the
 *           two helpers and of the tested bit are not established here, so the
 *           helper names stay address-anchored. One payload cmp of the 92-byte
 *           range proved this span byte-identical to the unlifted asm rows at
 *           0x801EF2A8 of emi/bmagic/magic046/03, 0x801F043C of
 *           emi/bmagic/magic067/03 and 0x800C1FA4 of emi/boss/boss036/16, none
 *           of which carries lifted source, so the statement order, operand
 *           forms and declaration style are copied from the exact sibling
 *           invokeHelpers4D978And4D290ThenDecrementWorkByte9AndInvokeHelper
 *           WhenZeroBmagicMagic12003_801F0044, whose 0x18-byte frame and two
 *           no-argument calls are the same and whose body differs in the second
 *           callee and in the work-byte operation that follows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callAdvanceWorkSet(
    void) {
  func_8014D978();
  func_8014D4E0();
  if (D_801462E8 & 4) {
    g_battle_work[2]++;
  }
}
