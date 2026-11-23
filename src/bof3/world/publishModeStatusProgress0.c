#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u16 D_801F4D68[]; /* @source 0x801F4D68 @kind table */
extern u8 g_ScenarioProgress;

/* @source 0x801F386C
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F4D68 and
 *           publishes it into the shared status halfword D_801490A8. On the
 *           same mode value it then stores 0 into the shared scenario progress
 *           byte g_ScenarioProgress when the mode reads 0 and 0xFF when it
 *           reads 1, and any other mode leaves that byte untouched. Takes no
 *           arguments and returns nothing; leaf, no frame.
 *           Constant/variance evidence: the exact sibling
 *           publishModeStatusFromTable4d64AndProgress14Or3World01Area06813_801F381C
 *           in this target supplied the statement order and operand forms --
 *           the signed mode read, the scaled target-local halfword index, the
 *           status publish and the two-case switch on that mode. That 80-byte
 *           body differs from this 92-byte span in the table base (0x801F4D64
 *           there, 0x801F4D68 here, both in the 0x801F0000 page so only the
 *           %lo word moves) and in the arm stores: the sibling's 0x14 and 3
 *           constants merge into one shared store fed through $v0, while this
 *           span's 0 and 0xFF arms each store at their own site, with $zero
 *           stored directly and 0xFF materialised into $v0 in the second
 *           compare's delay slot. Every literal above comes from these
 *           original bytes, and the 92-byte span's opening 28 bytes occur only
 *           in this payload, so no byte-identical copy exists to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusProgress0(
    void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F4D68[mode];
  switch (mode) {
  case 0:
    g_ScenarioProgress = 0;
    break;
  case 1:
    g_ScenarioProgress = 0xFF;
    break;
  default:
    break;
  }
}
