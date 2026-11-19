#include "bof3/bof3.h"

extern u16 D_801F4E08; /* @source 0x801F4E08 @kind unknown */
extern u8 D_801F4E0C;  /* @source 0x801F4E0C @kind unknown */

/* @source 0x801F3074
 * @behavior Overlay step handler: reads this overlay's recorded pad-match
 *           count D_801F4E08 - the halfword func_801F2E68 and func_801F2F98
 *           latch from the live pad-0x40 counter D_801F4E04 when the pan
 *           countdown D_801F4DFC expires - and selects the state index it
 *           stores into the byte at offset 0x01 of the work record published
 *           at the scratchpad pointer cell 0x1F800044, the byte the local
 *           dispatchers dispatchStateByte1Table3ff4World01Area06813 and
 *           dispatchStateByte1Table4004World01Area06813 index their handler
 *           tables with. While the recorded count is nonzero it clears the
 *           attempt byte D_801F4E0C and stores 0xB (11), the pan state
 *           stepCoordinatePairByFaceButtonAndArmState12World01Area06813 is
 *           reached from; while the recorded count is zero it stores 0x16
 *           (22). Both arms name the pointer cell through the same base
 *           variable, so the emitted byte stores coincide and the compiler
 *           merges them into the single store that follows the branch, each
 *           arm's state constant filling a branch delay slot. Leaf with no
 *           frame: it takes no arguments, makes no call and returns nothing.
 *           Shape evidence, no clone: the neighbour func_801F3764 was still on
 *           its asm row when this body was written, and it reads the same
 *           recorded count against the live counter, clears D_801F4E0C in its
 *           matching arm and stores 0x15 or 0x16 through the same pointer cell
 *           into the same record byte, so its statement order is followed here
 *           while this function's own test and literals come from these bytes;
 *           the same-target exact sibling
 *           countdownStepArmState14Or10ByPad40Else11World01Area06813 supplies
 *           the per-arm pointer-cell reload and merged-store spelling. No
 *           byte-identical sibling exists: a scan of the 398 extracted payloads
 *           under out/binaries found this 0x3C-byte span only at offset 0x474
 *           of this target, and its cell immediates are this overlay's own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armState11WhenRecordedPadMatchCountElse22World01Area06813_801F3074(void) {
  u8** slots;

  if (D_801F4E08 == 0) {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 0x16;
  } else {
    slots = SPAD_PTR_TABLE(u8);
    D_801F4E0C = 0;
    slots[0x11][0x01] = 0xB;
  }
}
