#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. The
 * countdown store and the word store share the one loaded pointer, while the
 * advance of the work byte reloads the cell because that byte store through the
 * loaded pointer may alias it. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801D0C70
 * @behavior Entry 0 of this overlay handler run at 0x801D1B1C, which the
 * overlay entry func_801D0C04 selects with the unsigned work byte at offset
 * +0x1 of the scratchpad work object published at pointer slot 0x1F800044 and
 * calls with no arguments: it re-arms that object countdown byte at +0x9 with
 * 0x10, zeroes the work accumulator word at +0xC through the same loaded
 * pointer, and then advances that work byte at +0x1 by one through a reloaded
 * pointer cell, so the next frame dispatches entry 1 (0x801D0CA0). It takes no
 * arguments, returns nothing and keeps no frame.
 *
 * Evidence: the first of the fourteen code-pointer words at 0x801D1B1C is
 * 0x801D0C70, so this body is element 0 of that run, and the run is what the
 * overlay entry indexes with the work byte at +0x1. The +0x9 byte and the +0xC
 * word are the cells the neighbouring exact lift
 * workWordTickCountdown
 * ticks and ramps by +0x10, and the span func_801D0D5C still ramps that word by
 * -0x10, which makes this body the reset that starts the run.
 *
 * Shape evidence: the 12-instruction body reads the pointer cell twice only
 * because the byte store through the first loaded pointer may alias it, the
 * same idiom the exact sibling 0x801D0CA0 uses for its re-arm and advance. No
 * other extracted payload holds this body: the 48 bytes are unique among the
 * 398 files under out/binaries.
 *
 * Representation notes from the shape sweep: the reload of the pointer cell
 * has to stand in the source before the accumulator store, which keeps both
 * loaded pointers live across that store and reproduces the original's $v1 and
 * $a0 pair. The natural order - accumulator store first, cell read only for the
 * advance - measured 8/13 instructions and 48->52 bytes, an extra load-delay
 * nop on one reused pointer and the cell load after the store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateWordArmScenario(
    void) {
  u8* work;
  u8* cursor;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  work[0x9] = 0x10;
  cursor = D_1F800044;
  *(s32*)(work + 0xC) = 0;
  cursor[0x1]++;
}
