#include "bof3/bof3.h"

void func_801FA38C(void);
void func_80196070(void);

/* @source 0x801F7DDC
 * @behavior Overlay per-frame handler: word 3 of the twelve-word in-image
 * code-pointer run based at 0x801FDDC0 (the word at 0x801FDDCC), which
 * dispatchWorkByte2HandlerScenarioScena0700_801F7B9C indexes with the scratchpad
 * work object's unsigned byte at offset 0x02, so it runs while that byte reads 3.
 * That same word is word 1 of the ten-word run based at 0x801FDDC8, so
 * dispatchWorkByte2HandlerScenarioScena0700_801F7D28 selects it as well, when
 * that byte reads 1. Those two run words are the only image words naming the
 * address, and no in-image jal reaches it.
 * It first runs this overlay's effect draw pass at 0x801FA38C with no arguments
 * and no result. That pass transforms 0x1E2 entries of the effect vertex state at
 * 0x800E595C and 0x800E686C around the work object's position words at 0x34,
 * 0x38 and 0x3C, takes its draw mode and texture page from GetTPage, and submits
 * 0x200 semi-transparent four-vertex polygons whose screen coordinates, UVs and
 * colour bytes come from the index table at 0x800E6FF4, the projected vertex
 * table at 0x800E686C and the colour table at 0x800E7FF4.
 * Only after that call does this body re-read the scratch work-object pointer
 * cell at 0x1F800044, because the pass may rewrite that cell, and it decrements
 * the work object's 32-bit word at 0x0C and stores the result unconditionally -
 * the store sits in the branch delay slot, so it happens on both arms. When the
 * stored value reaches zero it calls the shared work-area reset helper at
 * 0x80196070, which clears bytes 0x00 through 0x04 of the work object and so
 * returns the overlay to work state 0.
 * The word at 0x0C is the countdown the previous word of both runs,
 * statePairDistanceArm,
 * arms with 0x78 while it advances byte 0x02 to this handler.
 * It takes no arguments, returns nothing, reads and writes nothing else, and its
 * 0x18-byte frame only keeps $ra across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderSemiTransparentQuads(
    void) {
  u8* work;
  s32 count;

  func_801FA38C();
  work = SPAD_PTR_SLOT(u8, 0x44u);
  count = *(s32*)(work + 0x0C) - 1;
  *(s32*)(work + 0x0C) = count;
  if (count == 0) {
    func_80196070();
  }
}
