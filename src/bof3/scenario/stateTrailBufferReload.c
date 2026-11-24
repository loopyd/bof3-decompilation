#include "bof3/bof3.h"

/* Fixed-address state cells, both named by this target's own map: the
 * scratchpad work-pointer cell at 0x1F800044, and the unsigned halfword at
 * 0x800E4C1E in the main executable, outside this overlay's payload, whose
 * wider role is still unknown. Reads of the pointer cell keep their own named
 * loads, which is what keeps the countdown arm's two pointer reads separate
 * `lui`/`lw` pairs in the original bytes. */
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

/* @source 0x801F724C
 * @behavior Overlay per-frame handler, entry 3 of the eight-word byte-2
 * code-pointer run at 0x801FDD50 (its own word sits at 0x801FDD5C) that the
 * state handler at 0x801F700C indexes with the work object's unsigned byte at
 * +0x02. It first runs the trail rebuild at 0x801F88B8 on the trail buffer base
 * 0x800E48E0, then the trail draw pass at 0x801F8BE8 on that same base,
 * discarding both results. It then reads the shared unsigned halfword at
 * 0x800E4C1E, loads the scratchpad work pointer from slot 0x1F800044 while that
 * value is still live, and stores the halfword back plus 6 modulo 65536, where
 * the exact sibling of this body subtracts 12 from the same cell instead. It
 * then decrements the work object's countdown byte at +0x09 modulo 256, and
 * when the decremented count reaches zero it reads the scratchpad pointer slot
 * again, reloads that same countdown byte with the constant 0x20, reads the
 * pointer slot a third time and increments the work object's byte at +0x02, the
 * state machine's handler index, so that increment advances the run. The
 * countdown reload is the one statement no exact sibling carries; the
 * 0x801F8730 handler reloads its own countdown byte with that same constant.
 * The value 0x20 is materialised in the branch delay slot of the countdown test.
 * Takes no arguments and returns nothing; its 0x18-byte frame only keeps $ra
 * across the two calls. No in-image jal reaches the address: the run word at
 * 0x801FDD5C is the only image word naming it. Each read of the scratchpad
 * pointer cell at 0x1F800044 is its own named-cell load, so both reloads in the
 * countdown arm keep the original's separate `lui`/`lw` pairs instead of a
 * hoisted base. The 148 bytes share their statement order, operand forms and
 * offset spelling with the already-exact near-twin
 * stateTrailRebuildTick,
 * whose body differs only in the halfword literal and in that added countdown
 * reload, so the twin supplied the shape and this span's own bytes supplied the
 * literals.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateTrailBufferReload(void) {
  u16 value;
  u8* work;
  u8 countdown;

  func_801F88B8(PSX_PTR(u8, 0x800E48E0u));
  func_801F8BE8(PSX_PTR(u8, 0x800E48E0u));
  value = scenarioHalfword800;
  work = workPointerCell;
  scenarioHalfword800 = (u16)(value + 6);
  countdown = (u8)(work[9] - 1);
  work[9] = countdown;
  if (countdown == 0) {
    workPointerCell[9] = 0x20;
    workPointerCell[2]++;
  }
}
