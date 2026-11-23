#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u16 D_801F4498[]; /* @source 0x801F4498 @kind table */
extern u8 g_ScenarioProgress;

/* @source 0x801F3C44
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F4498 and
 *           publishes it into the shared status halfword D_801490A8. On the
 *           same mode value it then stores 1 into the shared scenario progress
 *           byte g_ScenarioProgress when the mode reads 0 and 0xA when it reads
 *           1, and any other mode leaves that byte untouched. Takes no
 *           arguments and returns nothing; leaf, no frame.
 *           Constant/variance evidence: every literal in the 0x50-byte body
 *           comes from these original bytes -- the mode byte cell 0x801490C7,
 *           the target-local table base 0x801F4498 (whose halfwords at offset
 *           zero read 0x68 and 0x21), the status cell 0x801490A8, the scenario
 *           progress cell 0x80146864 and the arm constants 1 and 0xA. The
 *           instruction-for-instruction shape is the exact sibling
 *           publishModeStatusFromTable449cAndProgress1Or0AWorld02Area08013_801F3C94
 *           in this target: a payload comparison shows this 80-byte span and
 *           that sibling differ in exactly two words, the table-base %lo
 *           immediate (0x4498 here, 0x449C there) and its own j immediate,
 *           which shifts by the 0x50-byte distance between the two spans. The
 *           span equality makes that body shape evidence, not a clone, and a
 *           scan of all 398 shipped payloads finds this span exactly once.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusTable4498(
    void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F4498[mode];
  switch (mode) {
  case 0:
    g_ScenarioProgress = 1;
    break;
  case 1:
    g_ScenarioProgress = 0xA;
    break;
  }
}
