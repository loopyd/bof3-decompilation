#include "bof3/bof3.h"

/* Shared overlay state byte at 0x8014933B, bound by this target's map. The
 * overlay setter 0x801F4118 writes 1 here and this step advances the dispatch
 * state byte only while it still reads 1. */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

/* Overlay draw routine at 0x801F3030, still unlifted assembly inside this
 * target, so only its prototype is declared. It takes one mode argument, whose
 * low byte it reads back off its own frame, projects the eight panel records
 * seeded at 0x801F412C through RotTransPers4 and appends them to the primitive
 * cursor at 0x8014598C. */
void func_801F3030(s32 mode);

/* @source 0x801F2CA4
 * @behavior Overlay step handler, entry 3 of the state dispatch table at
 *           0x801F4138 that the state-byte-+2 dispatcher 0x801F2C04 indexes.
 *           While the shared overlay state byte at 0x8014933B still reads 1 it
 *           loads the scratch work record published through the scratchpad
 *           pointer cell 0x1F800044 and advances that record's dispatch state
 *           byte +0x02 by one; the cell is loaded once for both the read and
 *           the store. It then always calls the overlay draw routine at
 *           0x801F3030 with mode 0. The function has a 0x18-byte frame keeping
 *           the return address at +0x10, takes no arguments and returns
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkStepIfStateOneThenDrawWorld04Area19613_801F2CA4(void) {
  u8* work;

  if (D_8014933B == 1) {
    work = SPAD_PTR_SLOT(u8, 0x44);
    work[2]++;
  }
  func_801F3030(0);
}
