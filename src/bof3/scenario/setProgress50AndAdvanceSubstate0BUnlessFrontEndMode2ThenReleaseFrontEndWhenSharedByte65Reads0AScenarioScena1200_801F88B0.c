#include "bof3/bof3.h"

/* Shared scenario progress byte in the main executable.
 * @source 0x80146864 @kind unknown
 */
extern u8 g_ScenarioProgress;

/* Shared front-end mode byte in the main executable: this handler's first arm
 * waits for it to stop reading 2.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Shared main-RAM byte this handler's second arm tests against 0x0A and
 * clears.
 * @source 0x80146865 @kind unknown
 */
extern u8 D_80146865;

/* Shared sub-state byte the two arms read and advance.
 * @source 0x80146875 @kind unknown
 */
extern u8 D_80146875;

/* Shared state byte the release arm clears.
 * @source 0x80146874 @kind unknown
 */
extern u8 D_80146874;

/* Shared front-end release helper at 0x8015C058. */
void func_8015C058(void);

/* @source 0x801F88B0
 * @behavior Overlay secondary-state handler: it is the word at 0x801FD1C8,
 * entry 1 of the ten-word handler table at 0x801FD1C4 that
 * dispatchSecondaryStateScenarioScena1200_801F886C (0x801F886C) indexes with
 * the signed shared state byte D_80146874, so it runs while that byte reads 1;
 * the table word is the only image reference to this address, and no jal or j
 * in the payload reaches it. It steps the shared sub-state byte D_80146875
 * through two values. Sub-state 0x0A, the value
 * requestPrimaryState1Substate10ScenarioScena1200_801FC7D4 (0x801FC7D4)
 * requests together with state 1, returns untouched while the shared front-end
 * mode byte D_80143BB0 reads 2 and otherwise stores 0x32 in the shared progress
 * byte g_ScenarioProgress (0x80146864) and advances the sub-state byte to 0x0B.
 * Sub-state 0x0B tests the shared main-RAM byte D_80146865 against 0x0A and
 * returns untouched when it differs; when it matches, the arm clears that byte,
 * runs the shared front-end release helper func_8015C058 (0x8015C058) and
 * clears the sub-state byte and the state byte D_80146874, returning the
 * overlay state machine to its idle state. Any other sub-state value returns
 * untouched. It takes no arguments, returns nothing, and its 0x18-byte frame
 * keeps $ra and $s0. The sub-state address is held in $s0 for the whole body
 * because both arms store through it after the mode and byte tests, which is
 * the shape the exact sibling
 * stepSharedModeStartAction1ThenReleaseOverlayOnPhaseChangeWorld02Area10513_801F3180
 * (0x801F3180 of emi/world02/area105/13) uses for its own mode byte, so this
 * seed holds it in a local pointer; every other shared byte store materialises
 * the 0x8014 page in $at on its own. The D_80146865 address reaches its load
 * and its clearing store through one base register with a zero offset.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setProgress50AndAdvanceSubstate0BUnlessFrontEndMode2ThenReleaseFrontEndWhenSharedByte65Reads0AScenarioScena1200_801F88B0(void) {
  u8 *substate;
  u8 *byte65;

  substate = &D_80146875;
  switch (*substate) {
  case 0xA:
    if (D_80143BB0 == 2) {
      break;
    }
    g_ScenarioProgress = 0x32;
    *substate = 0xB;
    break;
  case 0xB:
    byte65 = &D_80146865;
    if (*byte65 != 0xA) {
      break;
    }
    *byte65 = 0;
    func_8015C058();
    *substate = 0;
    D_80146874 = 0;
    break;
  }
}
