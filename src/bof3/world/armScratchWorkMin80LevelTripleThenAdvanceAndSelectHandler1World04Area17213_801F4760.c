#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void advanceScratchWorkLevelTripleAndAccumulatorsAtColorMarkerWorld04Area17213_801F47F0(
    void);

/* @source 0x801F4760
 * @behavior Arms the scratch work record published through the pointer cell at
 * 0x1F800044 the way the neighbouring armScratchWorkMin80LevelTriple does but
 * raises flag bit 0x20 before clearing bit 0x40: it sets flag bit 0x20 of
 * record byte 0, then clears flag bit 0x40, writes 1 into the 0x5C marker byte,
 * and writes -0x80 into the signed level bytes 0x5F, 0x5E and 0x5D in that
 * order through one load of the cell. It then advances the same record through
 * advanceScratchWorkLevelTripleAndAccumulatorsAtColorMarkerWorld04Area17213_801F47F0
 * and, after a fresh cell load, writes 1 into the handler byte at offset 0x04
 * that the local dispatcher indexes its handler table with, so the following
 * frame steps the record through the advance handler instead of this arming
 * entry. Takes no arguments; its 0x18-byte frame keeps $ra and $s0, which holds
 * the 1 that both byte stores write.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armScratchWorkMin80LevelTripleThenAdvanceAndSelectHandler1World04Area17213_801F4760(
    void) {
  u8* levels;

  D_1F800044[0] |= 0x20;
  D_1F800044[0] &= 0xbf;
  D_1F800044[0x5c] = 1;
  /* One load of the cell serves the whole level triple, which the original
   * writes high to low with a single -0x80 constant. */
  levels = D_1F800044;
  *(s8*)(levels + 0x5f) = -0x80;
  *(s8*)(levels + 0x5e) = -0x80;
  *(s8*)(levels + 0x5d) = -0x80;
  advanceScratchWorkLevelTripleAndAccumulatorsAtColorMarkerWorld04Area17213_801F47F0();
  D_1F800044[4] = 1;
}
