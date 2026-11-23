#include "bof3/bof3.h"

extern u16 D_801F4E04; /* @source 0x801F4E04 @kind unknown */
extern u16 D_801F4E08; /* @source 0x801F4E08 @kind unknown */
extern u8 D_801F4E0C;  /* @source 0x801F4E0C @kind unknown */

/* @source 0x801F3764
 * @behavior Overlay step handler for the pad-press timing window: reads the
 *           recorded pad-match count D_801F4E08 - the halfword func_801F2E68
 *           and func_801F2F98 latch from the live press counter D_801F4E04
 *           when the pan countdown D_801F4DFC expires - and that live counter
 *           itself, then selects the state index it stores into the byte at
 *           offset 0x01 of the work record published at the scratchpad pointer
 *           cell 0x1F800044, the byte the local dispatchers
 *           dispatchStateByte1Table3ff4World01Area06813 and
 *           dispatchStateByte1Table4004World01Area06813 index their handler
 *           tables with. While the live counter has moved outside the band
 *           one press either side of the recorded count it stores 0x16 (22),
 *           the overlay's give-up state; while the counter still lies inside
 *           that band it clears the attempt byte D_801F4E0C and stores 0x15
 *           (21). Both arms name the pointer cell through the same base
 *           variable, so the emitted byte stores coincide and the compiler
 *           merges them into the single store that follows the branch, the
 *           0x15 constant filling the delay slot of the second band test.
 *           Leaf with no frame: it takes no arguments, makes no call and
 *           returns nothing. The payload's handler tables at 0x801F3FF4 and
 *           0x801F4004 list this routine at entries 0x18 and 0x14.
 *           Seed evidence, no clone: the bare counter band and the two state
 *           literals come from these bytes. Two spellings of the band were
 *           measured first and both lost: writing the out-of-range case as
 *           the tested condition (live below the band or above it) also
 *           measured 15/22 instructions at 68.18% and laid the two arms out
 *           the other way round, and writing the in-range case as recorded
 *           minus one at most live and live at most recorded plus one kept
 *           that arm layout but measured the same 15/22 with the two loaded
 *           halfwords in the other registers. Reading the band with live on
 *           the left of both comparisons is the spelling these bytes show.
 *           The same-target sibling
 *           armRecordedPadMatch
 *           was measured exact and supplies the per-arm pointer-cell reload,
 *           the D_801F4E0C clear ahead of the merged store and the
 *           declaration style. No byte-identical sibling exists for the clone
 *           lever: a scan of the 398 extracted payloads under out/binaries
 *           found this 0x58-byte span only at offset 0xB64 of this target,
 *           because the three cell addresses are this overlay's own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armLiveWithinRecorded(void) {
  u8** slots;
  u16 recorded;
  u16 live;

  recorded = D_801F4E08;
  live = D_801F4E04;
  if (live >= recorded - 1 && recorded + 1 >= live) {
    slots = SPAD_PTR_TABLE(u8);
    D_801F4E0C = 0;
    slots[0x11][0x01] = 0x15;
  } else {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 0x16;
  }
}
