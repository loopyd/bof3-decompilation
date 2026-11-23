#include "bof3/bof3.h"

/* Overlay-local pointer cell holding the shared main-RAM effect object at
 * 0x800E5148 that the sibling run entry at 0x801F7638 publishes into it. The
 * cell sits in the payload's final data span (the Splat textbin block labelled
 * T_801FDE34) and has no target map row; the generated undefined_syms_auto.txt
 * row binds it. */
/* @source 0x801FDFBC @kind data */
extern u8* D_801FDFBC;

/* Quad helper at 0x801F97E0, the same draw call the sibling run members at
 * 0x801F76D0 and 0x801F77AC make: it picks one 0x20-byte-stride entry per table
 * pointer the object holds, fetches the texture page and submits a
 * semi-transparent Gouraud quad, walking the object's unsigned halfword at
 * +0x10 (clamped to 0xFF) as its loop bound. This caller discards its result. */
s32 func_801F97E0(u8* obj);

/* Shared main-executable work-area reset at 0x80196070, lifted exact as
 * clearWorkFlags under the ui module: it zeroes the work object's bytes
 * 0x00-0x04, the request this handler makes once its growing word passes the
 * limit. It takes no arguments and returns nothing. */
void func_80196070(void);

/* @source 0x801F7810
 * @behavior Entry 3, the last word of the four-word in-image code-pointer run
 * labelled D_801FDD8C at 0x801FDD8C (0x801F7638 / 0x801F76D0 / 0x801F77AC /
 * this function), which dispatchWorkByte2HandlerScenarioScena0700_801F75F4
 * indexes with the scratchpad work object's unsigned byte at offset 0x02, so it
 * runs while that byte reads three; no in-image `jal` reaches the address and
 * that run word at 0x801FDD98 is the only image reference to it. It loads the
 * effect-object pointer cell at 0x801FDFBC before it creates its frame, adds
 * 0x01000000 to that object's signed word at offset 0x08 - the word the run
 * entry at 0x801F7638 seeds with the work object's 16.16 distance, so this add
 * moves the ramp by 256.0 of that scale - and stores the sum back in the delay
 * slot of the call that follows. It then calls the quad helper at 0x801F97E0
 * with the object pointer as its only argument, so the object is drawn once per
 * tick with the distance it now carries. The call clobbers the registers, so
 * the pointer cell at 0x801FDFBC is read a second time afterwards; through that
 * read the function reloads the object's word at 0x08 and, when it reads
 * greater than 0x0FFFFFFF, calls the shared main-executable work-area reset at
 * 0x80196070. It never touches the work object's handler byte at 0x02, so the
 * reset is this run member's only way out. Takes no arguments, returns nothing,
 * and its 0x18-byte frame only keeps $ra at 0x10.
 *
 * Seed and representation notes. Written directly from this span's original
 * bytes with the statement order, operand forms, offset spelling and
 * declaration style of the sibling run members
 * tickObjectRampDrawQuadAndAdvanceHandlerScenarioScena0700_801F77AC (the
 * pointer-cell load before the frame, the in-place word update and the second
 * read of the cell after the call) and
 * stateEffectObjectDistance
 * (the effect object word at offset 0x08 that it seeds). No probe ran before
 * the first gate, which measured 23/23 instructions and 92/92 bytes: the word
 * is written and re-read through a signed 32-bit view, the limit test uses the
 * signed width that emits `slt`, and the 0x01000000 growth is the literal the
 * span stores. The clone lever was declined: a scan of the 398 payloads under
 * out/binaries for this 0x5C-byte span matched the span itself only, so no
 * byte-identical twin exists anywhere. The data and carve levers were
 * ineligible: the span opens with `addiu $sp` and `sw $ra` and closes with its
 * own `jr $ra` epilogue, and the run word at 0x801FDD98 is its only image
 * reference.
 * @status exact
 * @match 100.00
 * @residual none
 */
void expandObjectPastMax(
    void) {
  u8* obj = D_801FDFBC;

  *(s32*)(obj + 0x08) += 0x01000000;
  func_801F97E0(obj);
  if (*(s32*)(D_801FDFBC + 0x08) > 0x0FFFFFFF) {
    func_80196070();
  }
}
