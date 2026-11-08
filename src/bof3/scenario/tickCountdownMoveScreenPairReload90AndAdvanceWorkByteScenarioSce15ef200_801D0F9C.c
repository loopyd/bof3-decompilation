#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. The
 * three accesses before the guard share one loaded pointer, while each access
 * of the zero branch loads the cell again because the byte stores through the
 * loaded pointer may alias the cell. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801D0F9C
 * @behavior Entry 8 of the overlay's fourteen-word code-pointer run at
 * 0x801D1B1C, which the overlay entry func_801D0C04 selects with the unsigned
 * work byte at offset +0x1 of the scratchpad work object published at pointer
 * slot 0x1F800044 and calls with no arguments: it moves that object's
 * projected screen-pair halfwords at +0x2E and +0x30 by +0x15 and +9 through
 * the one loaded pointer, ticks its countdown byte at +0x9 down by one, and
 * leaves the +0x30 store for the guard branch delay slot. When the decremented
 * byte reaches zero it re-arms that byte with 0x5A and advances the same work
 * byte at +0x1 by one, so the next frame dispatches entry 9 (0x801D1008).
 * Both zero-branch accesses reload the pointer cell because the byte stores
 * through the loaded pointer may alias it. It takes no arguments, returns
 * nothing and keeps no frame.
 *
 * Evidence: the fourteen words from 0x801D1B1C name 0x801D0C70, 0x801D0CA0,
 * 0x801D0D00, 0x801D0D5C, 0x801D0DE8, 0x801D0E30, 0x801D0EC4, 0x801D0F68,
 * 0x801D0F9C, 0x801D1008, 0x801D1050, 0x801D10E8, 0x801D1118 and 0x801D11A4,
 * so this body is element 8 of that run. Entry 7 (0x801D0F68) names the same
 * two halfwords as the projected screen pair of this work object and lies at
 * the word before this one, the +0x9 byte is the countdown that entries 1, 2
 * and 7 of the same run tick and re-arm, and the +0x1 byte is the run's own
 * selector, so the pair moved here is that same projected screen pair.
 *
 * Shape: the guard-and-reload tail repeats the exact lift
 * add16ToWorkWordCAndTickCountdownReload32AdvanceWorkByte1ScenarioSce15ef200_801D0CA0
 * of this target instruction for instruction, with the two halfword moves in
 * place of its word move and 0x5A in place of its 0x20. The 0x6C-byte body has
 * no byte-identical sibling in this repository's exported asm, so the span is
 * not a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownMoveScreenPairReload90AndAdvanceWorkByteScenarioSce15ef200_801D0F9C(
    void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  *(u16*)(work + 0x2E) = *(u16*)(work + 0x2E) + 0x15;
  *(u16*)(work + 0x30) = *(u16*)(work + 0x30) + 9;
  if (--work[0x9] == 0) {
    D_1F800044[0x9] = 0x5A;
    D_1F800044[0x1]++;
  }
}
