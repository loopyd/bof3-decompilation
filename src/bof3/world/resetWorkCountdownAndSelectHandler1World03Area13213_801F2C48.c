#include "bof3/bof3.h"

/* Shared published work-pointer cell. Declared plain, not volatile: the one
 * reload this body shows follows the published store to the record byte at +9,
 * which may alias the cell itself, so the compiler re-materialises the cell
 * address and reloads the pointer with no volatile annotation needed. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F2C48
 * @behavior Entry 0 of the byte-1 handler pointer run based at 0x801F3DD8:
 *           loads the work record published at the scratchpad pointer cell
 *           0x1F800044, clears its countdown byte at +0x09 to zero, then reads
 *           that cell again and stores 1 in the record state byte at +0x01, so
 *           the next step of the exact same-target dispatcher
 *           dispatchStateByte1Table3dd8WorldWorld03Area13213_801F2C04 calls
 *           entry 1 of the run at 0x801F2C6C. Takes no arguments and returns
 *           nothing.
 *
 * Name evidence: the only in-image word naming 0x801F2C48 is the first word of
 * the pointer run at 0x801F3DD8, entry 0 of the table the dispatcher at
 * 0x801F2C04 indexes with the record byte at +0x01. The countdown and state
 * roles come from this body's own stores and offsets and from the exact
 * same-target entry 1 handler at 0x801F312C, which ticks the same byte at +0x09
 * and advances the same byte at +0x01.
 *
 * Shape evidence: a payload cmp of this span with the 0x24 bytes at 0x801F3288
 * in emi/world00/area008/13 found no differing word, and that twin
 * (resetScratchSelectMode1.c) carries an exact recorded status, so its
 * clear-countdown-then-select-1 statement order and its void result are the
 * seed here. The cell binding, the plain offset operand form and the reload
 * point are this target's own, taken from the exact
 * seedWorkWords34To3CAnd0CTo14DispatchCue207AndAdvanceWorkByte1World03Area13213_801F30C0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetWorkCountdownAndSelectHandler1World03Area13213_801F2C48(void) {
  u8* work;
  u8* state;

  work = D_1F800044;
  work[9] = 0;
  state = D_1F800044;
  state[1] = 1;
}
