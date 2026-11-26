#include "bof3/bof3.h"

/* Shared main-RAM flag bank of the main executable, outside this overlay's
 * payload, used only as the base argument of the shared bit clear helper. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Shared overlay state byte of the main executable, outside this overlay's
 * payload. The exact neighbour stateSharedMode2, entry 5 of the
 * handler table below, writes 2 to it; this entry 6 clears it. */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

/* Shared bit clear helper of the main executable, outside this overlay's
 * payload: clears the bit its second argument selects in the bank its first
 * argument addresses. */
extern void func_8015B5A8(void* flag_bank, u8 bit_index); /* @source 0x8015B5A8 */

/* @source 0x801F3F28
 * @behavior Entry 6 of the 25-entry overlay handler table at 0x801F6830 of
 *           this payload, whose entry 5 is the exact lift
 *           stateSharedMode2 and whose word at 0x801F6848
 *           publishes this span. Clears bit 0x46 of the shared main-RAM flag
 *           bank D_80144F28 through the shared bit clear helper func_8015B5A8
 *           and then stores zero into the shared overlay state byte
 *           D_8014933B. Takes no arguments and returns nothing; the 0x18-byte
 *           frame holds only $ra at +0x10, and no jal in this payload reaches
 *           the span, so its argument roles stay structural and none are
 *           taken.
 *
 * Name evidence: the first argument of the call is the shared flag bank
 * 0x80144F28 and the second is the bit index 0x46, the call shape this
 * repository's already-exact siblings record for the bit clear helper
 * func_8015B5A8 (for example clearSharedFlagBit4EWorld04Area17213_801F4324
 * with index 0x4E and clearScenarioFlagBit3CAndQueueCue203World03Area11913_801F4554
 * with 0x3C). The trailing store clears 0x8014933B, the shared overlay state
 * byte this same target's neighbours seed with 1 and 2
 * (stateSharedMode2, stateSharedMode1), which is the
 * clearOverlayStateByte* family role. The body itself is byte-identical to the
 * exact cross-target twin clearSharedFlagBit46ThenClearOverlayStateByte
 * World03Area14813_801F39A4: 0x30 payload bytes compare equal word for word.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateResetFlagMode(void) {
  func_8015B5A8(&D_80144F28, 0x46);
  D_8014933B = 0;
}
