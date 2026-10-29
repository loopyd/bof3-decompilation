#include "bof3/bof3.h"

extern void (*D_801F5804[])(void);

/* @source 0x801F3E0C
 * @behavior Overlay step dispatch: reads the work-record state byte at offset
 * 0x01 of the record published at the scratchpad pointer slot 0x44 and calls
 * the handler pointer at that byte index of the local handler table based at
 * 0x801F5804. That table's row in the shipped image reads, in order,
 * 0x801F3E50, 0x801F3E8C, 0x801F3F78, 0x801F40C8 and 0x801F41B0 followed by a
 * zero word at 0x801F5818 (raw .word row inside the trailing asm blob of this
 * overlay). The already-exact same-target sibling
 * dispatchStateByte1Table57a4World03Area13413_801F3560 is the same dispatch
 * through the same state byte and the table based at 0x801F57A4. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte1Table5804World03Area13413_801F3E0C(void) {
  D_801F5804[SPAD_PTR_SLOT(u8, 0x44)[1]]();
}
