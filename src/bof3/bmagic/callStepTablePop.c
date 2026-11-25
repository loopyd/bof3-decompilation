#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The compiled original re-materializes the cell address for every
 * access, so the cell is reached through this target-map symbol rather than
 * through a constant address. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Work byte 2 step-handler run for the byte-1 phase this span serves, indexed
 * at its own base 0x801F0104: entry 0 = 0x801EF090, entry 1 = 0x801EF174,
 * entry 2 = 0x801EF21C, entry 3 = 0x801EF2C4, the byte-2 step handlers around
 * this span. The base is an interior word of the overlay code pointer run that
 * starts at 0x801F00F8, and no map row owns it, so the reference is bound by
 * the target's generated undefined-symbol table. */
extern void (*D_801F0104[])(void); /* @source 0x801F0104 @kind data */

/* In-overlay helper: pushes the GTE matrix stack, then projects the work
 * record's position pair and sets the rotation, translation and combined
 * matrices (PushMatrix / RotTrans / RotMatrix / MulMatrix2 / SetRotMatrix /
 * SetTransMatrix) in its 0x50-byte frame. Still an asm row in this target, so
 * its role stays address-anchored here. */
extern void func_801EF3B0(void); /* @source 0x801EF3B0 @kind function */

/* In-overlay helper called between the transform install and the matrix pop.
 * Still an asm row in this target, so its role stays address-anchored. */
extern void func_801EF4C4(void); /* @source 0x801EF4C4 @kind function */

/* @source 0x801EF018
 * @behavior Dispatches one frame of the overlay work object through the work
 *           byte 2 step table at 0x801F0104: the work-object pointer cell at
 *           0x1F800044 is read before the frame adjustment and its unsigned
 *           byte at offset 2, scaled by four, selects the code pointer at
 *           0x801F0104 plus that offset, which is invoked with no arguments.
 *           The table is indexed where it lives, so no table word is copied
 *           into the frame, and the selector is not bounds-checked. After the
 *           handler returns the pointer cell is read a second time and, when
 *           the work object's byte 0 is non-zero, the helper at 0x801EF3B0,
 *           the helper at 0x801EF4C4 and then PopMatrix at 0x80178C18 run in
 *           that order, each call's delay slot a nop. It takes no arguments
 *           and returns nothing; its 0x18-byte frame keeps only $ra at 0x10,
 *           and it writes no state. It is entry 0 of the in-image code pointer
 *           run at 0x801F00F8 that the byte-1 dispatcher at 0x801EEFD4
 *           indexes, so no jal in the payload reaches it. Bytes
 *           0x801EF018..0x801EF08F are not identical to the exact lift
 *           callHandlerInvokeNonzero:
 *           the guard body carries three calls there instead of one and the
 *           indexed base is 0x801F0104 instead of 0x801F0114, so the earlier
 *           lift supplied this body's statement order, operand forms and cell
 *           binding while each literal here comes from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callStepTablePop(
    void) {
  u8 *work = g_battle_work;

  D_801F0104[work[2]]();
  work = g_battle_work;
  if (work[0] != 0) {
    func_801EF3B0();
    func_801EF4C4();
    PopMatrix();
  }
}
