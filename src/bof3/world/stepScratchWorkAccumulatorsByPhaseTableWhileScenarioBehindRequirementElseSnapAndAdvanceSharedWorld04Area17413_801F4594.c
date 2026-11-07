#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */
extern u16* D_80148210; /* @source 0x80148210 @kind unknown */
extern s8 D_801F6754[]; /* @source 0x801F6754 @kind table */
extern u8 g_ScenarioProgress;

/* @source 0x801F4594
 * @behavior Per-frame step of the scratch work record published through the
 * scratchpad pointer cell 0x1F800044, gated on the scenario progress byte: it
 * reads the scenario requirement byte at 0x2 of the record table reached through
 * word 0x124 of the object published through 0x80146250, indexed by the halfword
 * at 0x0A of the shared record published through 0x80148210, and while
 * g_ScenarioProgress reads below that requirement it steps the record by the
 * signed 16-entry phase table at 0x801F6754, whose values are selected by the
 * phase byte at 0x0A masked to four bits: it adds the selected entry scaled by
 * 0x800 to the record word at 0x34, subtracts the same scaled entry from the
 * word at 0x38, decrements the phase byte, and consumes two units of the
 * halfword at 0x122 of the record published through 0x80146250. Once the
 * progress byte reaches the requirement it instead clears the low 15 bits of the
 * record words at 0x34 and 0x38 and advances the shared record halfword at 0x0A
 * by one. Takes no arguments, calls nothing and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepScratchWorkAccumulatorsByPhaseTableWhileScenarioBehindRequirementElseSnapAndAdvanceSharedWorld04Area17413_801F4594(
    void) {
  u8* obj;
  u16* shared;
  u8* records;
  u8* entry;

  obj = D_80146250;
  shared = D_80148210;
  records = *(u8**)(obj + 0x124);
  entry = records + shared[5] + 2;
  if (g_ScenarioProgress < *entry) {
    s32 step;
    volatile u8* work = D_1F800044;
    u8* counter;

    /* The original re-reads the phase byte for each of its three uses even
     * though no store separates the first two reads, so those reads are
     * volatile; the record words and the shared record stay plain because the
     * original lets the scheduler interleave those accesses. */
    *(s32*)(work + 0x34) += D_801F6754[work[0x0a] & 0x0f] << 11;
    step = D_801F6754[work[0x0a] & 0x0f] << 11;
    work[0x0a] = work[0x0a] - 1;
    counter = D_80146250;
    *(s32*)(work + 0x38) -= step;
    *(u16*)(counter + 0x122) -= 2;
  } else {
    u8* work = D_1F800044;

    *(u32*)(work + 0x34) &= 0xffff8000;
    *(u32*)(work + 0x38) &= 0xffff8000;
    shared[5] += 1;
  }
}
