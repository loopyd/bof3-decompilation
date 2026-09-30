#include "bof3/bof3.h"

extern void (*D_801F6270[])(void);

/* @source 0x801F4034
 * @behavior Overlay step dispatch: reads the non-volatile scratchpad work
 *           pointer cell at 0x1F800044, takes that work record's dispatch byte
 *           at offset 0x01 as its index and calls the handler pointer at that
 *           index of the local handler table based at 0x801F6270. The table's
 *           shipped row begins 0x801F4078, 0x801F40D4 (both handler boundaries
 *           of this overlay) and then runs into the overlay's byte data at
 *           0x801F6278. Same shape as the already-exact sibling
 *           dispatchStateByte1Table57a4World03Area13413_801F3560, and the twin
 *           of this target's 0x801F3E04, which dispatches the same work byte
 *           through the table based at 0x801F6268. Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte1Table6270World02Area07714_801F4034(void) {
  D_801F6270[SPAD_PTR_SLOT(u8, 0x44)[1]]();
}
