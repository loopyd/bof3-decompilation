#include "bof3/bof3.h"

extern u16 D_80143F00; /* @source 0x80143F00 @kind unknown */
void drawTranslucentGouraudRingOfSixteenQuadsAroundScreenCenterFadingFromBlackToWhiteScenarioScena0800_801F9688(
    void);

/* @source 0x801F9618
 * @behavior Calls the overlay-local translucent Gouraud ring emitter at
 * 0x801F9688, then increments unsigned byte 1 of the work object in scratchpad
 * pointer slot 0x44 modulo 256 unless the unsigned halfword at 0x80143F00
 * equals 2. Takes no arguments and returns no value; the halfword's role is
 * unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperThenConditionallyAdvanceWorkByte1ScenarioScena0800_801F9618(void) {
  drawTranslucentGouraudRingOfSixteenQuadsAroundScreenCenterFadingFromBlackToWhiteScenarioScena0800_801F9688();
  if (D_80143F00 != 2) {
    SPAD_PTR_SLOT(u8, 0x44)[1]++;
  }
}
