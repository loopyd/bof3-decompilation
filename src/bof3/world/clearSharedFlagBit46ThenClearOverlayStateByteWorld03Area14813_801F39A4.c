#include "bof3/bof3.h"

extern u8 D_80144F28;
extern u8 D_8014933B;

void func_8015B5A8(void* flag_bank, u8 bit_index);

/* @source 0x801F39A4
 * @behavior Overlay handler: clears bit 0x46 of the shared main-RAM flag bank
 *           at 0x80144F28 through the shared bit-clear helper func_8015B5A8
 *           and then clears the shared overlay state byte at 0x8014933B to
 *           zero. Takes no arguments and returns nothing; its 0x18-byte frame
 *           only keeps $ra across the call.
 *
 * Name evidence: the first argument of the call is the shared flag bank
 * 0x80144F28 and the second is the bit index 0x46, the call shape this
 * repository's already-exact siblings record for the bit-clear helper
 * func_8015B5A8 (for example clearSharedFlagBit4EWorld04Area17213_801F4324
 * with index 0x4E and clearScenarioFlagBit3CAndQueueCue203World03Area11913_801F4554
 * with 0x3C). The trailing store clears 0x8014933B, the overlay state byte
 * this same target's neighbours seed with 1 and 2
 * (setWorld03Area14813_801F3990, setWorld03Area14813_801F3A0C) and the
 * clearOverlayStateByte* family zeroes. No in-image caller exists: the
 * pointer 0x801F39A4 appears nowhere in this payload or in its incbin data
 * blob, so like its unindexed neighbours this handler is reached from a
 * dispatch table in another overlay sharing the same VRAM window.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedFlagBit46ThenClearOverlayStateByteWorld03Area14813_801F39A4(void) {
  func_8015B5A8(&D_80144F28, 0x46);
  D_8014933B = 0;
}
