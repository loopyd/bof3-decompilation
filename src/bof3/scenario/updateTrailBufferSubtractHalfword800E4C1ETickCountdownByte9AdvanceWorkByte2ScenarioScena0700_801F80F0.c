#include "bof3/bof3.h"

/* Fixed-address state cells, both named by this target's own map: the
 * scratchpad work-pointer cell at 0x1F800044, and the unsigned halfword at
 * 0x800E4C1E in the main executable, outside this overlay's payload, whose
 * wider role is still unknown. Reads of the pointer cell keep their own named
 * loads, which is what keeps the countdown arm's reload a separate `lui`/`lw`
 * pair in the original bytes. */
extern u8* workPointerCell; /* @source 0x1F800044 @kind unknown */
extern u16 scenarioHalfword800; /* @source 0x800E4C1E @kind unknown */

/* Trail rebuild at 0x801F88B8, still an `asm` row when this lift was written:
 * it shifts the trail records and recomputes their projected points and
 * headings. A payload cmp showed its 0x330 bytes equal to the scena08 copy at
 * 0x801F8D98 except for the five intra-body jump words the two placements
 * force. */
void func_801F88B8(u8* buffer);

/* Trail draw pass at 0x801F8BE8, still an `asm` row when this lift was written:
 * it consumes those records and submits primitives, reaching GetTPage,
 * SetDrawMode, SetPolyG4 and SetSemiTrans. A payload cmp showed its 0x358
 * bytes equal to the scena08 copy at 0x801F90C8 except for the two
 * average-helper jal words. */
void func_801F8BE8(u8* buffer);

/* @source 0x801F80F0
 * @behavior Overlay per-frame handler, entry 8 of the twelve-word in-image
 * code-pointer run based at 0x801FDDC0 (the word at 0x801FDDE0) that
 * callWorkByte2Step8 indexes with the work
 * object's unsigned byte at +0x02, so it runs while that byte reads 8. It first
 * runs the trail rebuild at 0x801F88B8 on the trail buffer base 0x800E48E0,
 * then the trail draw pass at 0x801F8BE8 on that same base, discarding both
 * results. It then reads the shared unsigned halfword at 0x800E4C1E, loads the
 * scratchpad work pointer from slot 0x1F800044 while that value is still live,
 * stores the halfword back minus 12 modulo 65536, decrements the work object's
 * countdown byte at +0x09 modulo 256, and when the decremented count reaches
 * zero reads the scratchpad pointer slot again and increments the work object's
 * byte at +0x02, the state machine's handler index, so that increment advances
 * the run. Takes no arguments and returns nothing; its 0x18-byte frame only
 * keeps $ra across the two calls. No in-image jal reaches the address: the run
 * word at 0x801FDDE0 is the only reference to it. Each read of the scratchpad
 * pointer cell at 0x1F800044 is its own named-cell load, so the reload in the
 * countdown arm keeps the original's separate `lui`/`lw` pair instead of a
 * hoisted base. The 132 bytes are byte-identical to the already-exact lift
 * stateTrailRebuildTick
 * (a payload cmp of the two 0x84-byte spans was equal word for word, both `jal`
 * immediates included), so that lift supplied the statement order, operand
 * forms and declaration style while this span's own address and run word are
 * written here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void updateTrailBufferSubtractHalfword800E4C1ETickCountdownByte9AdvanceWorkByte2ScenarioScena0700_801F80F0(void) {
  u16 value;
  u8* work;
  u8 countdown;

  func_801F88B8(PSX_PTR(u8, 0x800E48E0u));
  func_801F8BE8(PSX_PTR(u8, 0x800E48E0u));
  value = scenarioHalfword800;
  work = workPointerCell;
  scenarioHalfword800 = (u16)(value - 12);
  countdown = (u8)(work[9] - 1);
  work[9] = countdown;
  if (countdown == 0) {
    workPointerCell[2]++;
  }
}
