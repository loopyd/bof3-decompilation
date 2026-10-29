#include "bof3/bof3.h"

extern void (*D_801F57A4[])(void);

/* @source 0x801F3560
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work
 * record published at the scratchpad pointer cell 0x1F800044 and calls the
 * handler pointer at that byte index of the local handler table based at
 * 0x801F57A4 (whose entries are overlay handler addresses 0x801F35A4 and up).
 * Same shape as the already-exact sibling
 * dispatchStateByte1Table6894World03Area12114_801F45E0, and the structural
 * twin of this target's first boundary 0x801F2C04, which dispatches the same
 * state byte through the table based at 0x801F5744. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte1Table57a4World03Area13413_801F3560(void) {
  D_801F57A4[SPAD_PTR_SLOT(u8, 0x44)[1]]();
}
