#include "bof3/bof3.h"

/* Fixed-address state cell named by this target's own map: the scratchpad
 * work-pointer cell at 0x1F800044. Each source-level read of it keeps its own
 * named-cell load, because a store through the pointer it returned could write
 * that very cell, so this body reads the cell three times: once for the
 * arithmetic, once for the two end tests and once for the handler advance. */
extern u8* workPointerCell; /* @source 0x1F800044 @kind unknown */

/* Trail rebuild at 0x801F88B8, still an `asm` row in this target's map: it
 * shifts the trail records and recomputes their projected points and headings.
 * The exact sibling at 0x801F7EF8 recorded a payload cmp of the 0x330 bytes
 * against the scena08 copy at 0x801F8D98, equal except for the five intra-body
 * jump words the two placements force. */
void func_801F88B8(u8* buffer);

/* Trail draw pass at 0x801F8BE8, still an `asm` row in this target's map: it
 * consumes those records and submits primitives, reaching GetTPage,
 * SetDrawMode, SetPolyG4 and SetSemiTrans. The exact sibling at 0x801F7EF8
 * recorded a payload cmp of the 0x358 bytes against the scena08 copy at
 * 0x801F90C8, equal except for the two average-helper jal words. */
void func_801F8BE8(u8* buffer);

/* Seeded from the exact sibling updateTrailBufferAddWordsCAnd10And14ToWords34-*
 * And38And3CSetCountdownByte9AtTargetAdvanceWorkByte2ScenarioScena0700_801F7EF8
 * (0x801F7EF8, this target): that body has this one's trail call spelling, its
 * `u8*` work pointer over the named scratchpad cell, its word-at-offset stores
 * and its `0x2F`-into-byte-9 tail. This span's own bytes supplied the step
 * values, the fields it touches, the end-test offsets and the arm order. The
 * `0x38` statement is a store of the value just loaded from that same word in
 * both this span and the family's seeding member 0x801F7050 (`lw $a0,0x38`
 * then `sw $a0,0x38`, no add between), so its zero step is written as `+= 0`
 * and the original keeps the load/store pair.
 *
 * The clone lever is declined: one scan of every extracted payload under
 * out/binaries for this 0xC8-byte span, with the two `jal` words masked, matched
 * this overlay alone at payload offset 0x4F8, and all nine other prologue-
 * anchored hits diverged from span offset 0x20 onward. The range is code, not
 * data (it opens with `addiu $sp,$sp,-0x18`, closes with `jr $ra`, and the run
 * word at 0x801FDD54 is its only consumer), so the data and carve levers are
 * declined as well.
 *
 * @source 0x801F70F8
 * @behavior Overlay per-frame handler, entry 1 of the eight-word in-image
 * code-pointer run labelled D_801FDD50 at 0x801FDD50 that the state handler at
 * 0x801F700C indexes with the work object's unsigned byte at +0x02, so this
 * handler runs while that byte reads 1. It runs the trail rebuild at 0x801F88B8
 * on the trail buffer base 0x800E48E0 and then the trail draw pass at 0x801F8BE8
 * on that same base, discarding both results. It then loads the scratchpad work
 * pointer from slot 0x1F800044 and, through that saved pointer, subtracts
 * 0x10000 from the work word at +0x34, stores the work word at +0x38 back with
 * the value just loaded from it, and adds the work word at +0x14 into the work
 * word at +0x3C; the work word at +0x14 is then read again and, when it reads
 * non-zero, gains 0x800000. Finally it re-reads the pointer slot and, when that
 * pointer's word at +0x34 equals its word at +0x0C and its word at +0x38 equals
 * its word at +0x10, stores 0x2F into the work object's byte at +0x09 and, through
 * a third read of the pointer slot, increments its byte at +0x02 so the run moves
 * on to entry 2. Takes no arguments, returns nothing, and its 0x18-byte frame
 * only keeps $ra across the two calls; no in-image jal reaches the address, whose
 * only reference is the run word at 0x801FDD54.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateTrailBufferSubtract(
    void) {
  u8* work;
  u8* next;

  func_801F88B8(PSX_PTR(u8, 0x800E48E0u));
  func_801F8BE8(PSX_PTR(u8, 0x800E48E0u));
  work = workPointerCell;
  *(u32*)&work[0x34] -= 0x10000;
  *(u32*)&work[0x38] += 0;
  *(u32*)&work[0x3C] += *(u32*)&work[0x14];
  if (*(u32*)&work[0x14] != 0) {
    *(u32*)&work[0x14] += 0x800000;
  }
  next = workPointerCell;
  if (*(u32*)&next[0x34] == *(u32*)&next[0x0C] && *(u32*)&next[0x38] == *(u32*)&next[0x10]) {
    next[9] = 0x2F;
    workPointerCell[2]++;
  }
}
