#include "bof3/bof3.h"

/* Fixed-address state cell named by this target's own map: the scratchpad
 * work-pointer cell at 0x1F800044. Each source-level read of it keeps its own
 * named-cell load, because a store through the pointer it returned could write
 * that very cell, so the handler-advance arm's pointer read is a separate
 * `lui`/`lw` pair from the one saved pointer the arithmetic runs through. */
extern u8* scenarioScena0700Work; /* @source 0x1F800044 @kind unknown */

/* Trail rebuild at 0x801F88B8, still an `asm` row: it shifts the trail records
 * and recomputes their projected points and headings. A payload cmp showed its
 * 0x330 bytes equal to the scena08 copy at 0x801F8D98 except for the five
 * intra-body jump words the two placements force. */
void func_801F88B8(u8* buffer);

/* Trail draw pass at 0x801F8BE8, still an `asm` row: it consumes those records
 * and submits primitives, reaching GetTPage, SetDrawMode, SetPolyG4 and
 * SetSemiTrans. A payload cmp showed its 0x358 bytes equal to the scena08 copy
 * at 0x801F90C8 except for the two average-helper jal words. */
void func_801F8BE8(u8* buffer);

/* Seeded from the already-exact cross-target sibling
 * integratePositionPairUntilTargetThenResetState (0x801E352C, battle/03): that
 * 0xAC-byte body has this one's statement order, offsets and both `bne` tests,
 * adding the same work words 0x0C/0x10 into 0x34/0x38 and then comparing that
 * pair against the target pair 0x18/0x1C, and it re-reads 0x34 from memory for
 * the test after storing it. This target's own 0x801F805C supplied the trail
 * call spelling and the named work cell. The clone lever is declined: a scan of
 * every shipped payload for this span with only the two `jal` words masked
 * matched this overlay alone, at payload offset 0x12F8, so no byte-identical
 * sibling exists. The range is code, not data (it opens with
 * `addiu $sp,$sp,-0x18`, closes with `jr $ra`, and the run word at 0x801FDDD4
 * is its only consumer), so the data and carve levers are declined too.
 *
 * @source 0x801F7EF8
 * @behavior Overlay per-frame handler: entry 5 of the twelve-word in-image
 * code-pointer run based at 0x801FDDC0 (the word at 0x801FDDD4), which
 * dispatchWorkByte2HandlerScenarioScena0700_801F7B9C indexes with the
 * scratchpad work object's unsigned byte at offset 0x02, so this handler runs
 * while that byte reads 5. It runs the trail rebuild at 0x801F88B8 on the trail
 * buffer base 0x800E48E0, then the trail draw pass at 0x801F8BE8 on that same
 * base, discarding both results. It then loads the work-object pointer cell at
 * 0x1F800044 once and, through that saved pointer, adds the work object's word
 * at 0x0C into its word at 0x34, its word at 0x10 into its word at 0x38 and its
 * word at 0x14 into its word at 0x3C. When the updated word at 0x34 again reads
 * equal to the target word at 0x18 and the updated word at 0x38 again reads
 * equal to the target word at 0x1C, it stores 0x2F into the work object's byte
 * at offset 0x09 through that same saved pointer, then re-reads the pointer cell
 * and increments the work object's byte at offset 0x02, the state machine's
 * handler index, through the re-read pointer. Both comparisons re-read the words
 * they just stored instead of reusing the computed registers, and the 0x2F is
 * materialised in the branch delay slot of the second comparison. Takes no
 * arguments and returns nothing, and its 0x18-byte frame only keeps $ra across
 * the two calls. No in-image jal reaches the address: the run word at
 * 0x801FDDD4 is the only image word naming it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateTrailBufferTarget(void) {
  u8* work;

  func_801F88B8(PSX_PTR(u8, 0x800E48E0u));
  func_801F8BE8(PSX_PTR(u8, 0x800E48E0u));
  work = scenarioScena0700Work;
  *(u32*)&work[0x34] += *(u32*)&work[0x0C];
  *(u32*)&work[0x38] += *(u32*)&work[0x10];
  *(u32*)&work[0x3C] += *(u32*)&work[0x14];
  if (*(u32*)&work[0x34] == *(u32*)&work[0x18] && *(u32*)&work[0x38] == *(u32*)&work[0x1C]) {
    work[9] = 0x2F;
    scenarioScena0700Work[2]++;
  }
}
