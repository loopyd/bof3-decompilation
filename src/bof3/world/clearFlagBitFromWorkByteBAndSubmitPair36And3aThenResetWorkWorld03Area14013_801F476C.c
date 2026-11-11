#include "bof3/bof3.h"

/* @source 0x1F800044 @kind data */
extern u16 *g_WorkRecordWorld03Area14013;
/* @source 0x80144F28 @kind data */
extern u8 D_80144F28;
/* @source 0x8015B5A8 */
extern void func_8015B5A8(void *flag_bank, u8 bit_index);
/* @source 0x8015507C */
extern void func_8015507C(s32 arg0, s32 arg1);
/* @source 0x80196070 */
extern void func_80196070(void);

/* @source 0x801F476C
 * @behavior Overlay handler: entry 2, the last of the three-entry handler run
 *           at 0x801F4AC4 that this target's dispatched state byte at +0x01
 *           selects, so no jal reaches this body. It reads the scratchpad work
 *           pointer published at 0x1F800044 and clears the flag bit that work
 *           byte +0x0B indexes in the main-RAM flag bank at 0x80144F28 through
 *           the shared bit-clear helper 0x8015B5A8, re-reads the pointer cell,
 *           passes the signed halfwords at +0x36 and +0x3A to the shared
 *           request helper 0x8015507C, and ends by calling the shared
 *           work-area reset helper 0x80196070, which clears work bytes
 *           0x00..0x04. Takes no arguments and returns nothing; its 0x18-byte
 *           frame only keeps $ra at +0x10, and the first pointer load precedes
 *           the frame creation.
 *
 *           Name evidence: helper 0x8015B5A8 clears a bit of the flag bank its
 *           first argument names, the form the flag-clearing family in this
 *           repository uses (for example
 *           clearSharedFlagBit46ThenClearOverlayStateByteWorld03Area14813_801F39A4
 *           with the same bank 0x80144F28); here its index is the work byte at
 *           +0x0B, so that field stays address-anchored as a flag-bit index
 *           rather than an asserted role. The +0x36/+0x3A pair is the pair this
 *           target's handler func_801F34F4 passed to 0x8015507C, and 0x80196070
 *           is the reset helper this repository's overlay handlers end on
 *           (advanceWorkBytes9And10WithTableCallsThenResetAt49World02Area10613_801F2DDC,
 *           resetWorkFlagsOnZeroRefreshScenarioScena0800_801F7A9C). The
 *           trailing request helper keeps its unknown role, addressed.
 *
 *           Shape evidence: sibling func_801F34F4, at 0x801F34F4 of this
 *           payload, was lifted to
 *           src/bof3/world/advanceWorkHalfword3eBy10AndStepHalfwordPair36And3aAt80ThenAdvanceWorkByte2AndDrawWorld03Area14013_801F34F4.c
 *           with a measurement of 100.00 and supplied the statement order of
 *           the reload, the pair pass and the trailing call, plus the
 *           pointer-local spelling for the pointer cell at 0x1F800044; the
 *           reload-per-call grouping of the same target's
 *           armCountdownByte9To210AndAdvanceWorkByte1World03Area14013_801F46E4
 *           supplied the second load. A scan of every payload under
 *           out/binaries with jal target words and immediate fields masked
 *           found this 0x58-byte body only at 0x801F476C of this payload, so
 *           clone was declined and no byte-identical sibling exists.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearFlagBitFromWorkByteBAndSubmitPair36And3aThenResetWorkWorld03Area14013_801F476C(
    void) {
  u16 *work;

  work = g_WorkRecordWorld03Area14013;
  func_8015B5A8(&D_80144F28, ((u8 *)work)[0xB]);
  work = g_WorkRecordWorld03Area14013;
  func_8015507C((s16)work[0x1B], (s16)work[0x1D]);
  func_80196070();
}
