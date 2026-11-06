#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */
extern s8 D_801F6780[]; /* @source 0x801F6780 @kind table */
extern u8 g_ScenarioProgress;

/* @source 0x801F491C
 * @behavior While the shared scenario progress byte g_ScenarioProgress reads
 * fewer than 7, steps the work record published through the scratchpad pointer
 * cell 0x1F800044 by the signed 16-entry phase table at 0x801F6780, whose
 * values repeat the pattern -1, 0, 1, 0: it adds the selected entry scaled by
 * 0x800 to the record word at 0x34, subtracts the same scaled entry from the
 * word at 0x38, decrements the phase byte at 0x0A that indexes the table, and
 * consumes two units of the halfword at 0x122 of the record published through
 * the global pointer cell 0x80146250. Once the progress byte reads 7 or more it
 * instead clears the low 15 bits of the record words at 0x34 and 0x38. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepScratchWorkAccumulatorsByPhaseTableAndSnapWorld04Area17213_801F491C(void) {
  s32 step;

  if (g_ScenarioProgress < 7) {
    volatile u8* work = D_1F800044;
    u8* shared;

    /* The original re-reads the phase byte for each of its three uses even
     * though no store separates the first two reads, so those reads are
     * volatile; the record words and the shared record stay plain because the
     * original lets the scheduler interleave those accesses. */
    *(s32*)(work + 0x34) += D_801F6780[work[0x0a] & 0x0f] << 11;
    step = D_801F6780[work[0x0a] & 0x0f] << 11;
    work[0x0a] = work[0x0a] - 1;
    shared = D_80146250;
    *(s32*)(work + 0x38) -= step;
    *(u16*)(shared + 0x122) -= 2;
  } else {
    u8* work = D_1F800044;

    *(u32*)(work + 0x34) &= 0xffff8000;
    *(u32*)(work + 0x38) &= 0xffff8000;
  }
}
