#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u16 D_801F449C[]; /* @source 0x801F449C @kind table */
extern u8 g_ScenarioProgress;

/* @source 0x801F3C94
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F449C and
 *           publishes it into the shared status halfword D_801490A8. On the
 *           same mode value it then stores 1 into the shared scenario progress
 *           byte g_ScenarioProgress when the mode reads 0 and 0xA when it reads
 *           1, and any other mode leaves that byte untouched. Takes no
 *           arguments and returns nothing; leaf, no frame.
 *           Constant/variance evidence: every literal in the 0x50-byte body
 *           comes from these original bytes -- the mode byte cell 0x801490C7,
 *           the target-local table base 0x801F449C (whose halfwords at offset
 *           zero read 0x68 and 0x21), the status cell 0x801490A8, the scenario
 *           progress cell 0x80146864 and the arm constants 1 and 0xA. The
 *           instruction-for-instruction shape is the exact sibling
 *           publishModeStatusFromTable4494AndProgress1Or0AWorld02Area08013_801F3BF4
 *           in this target, which indexes its own table 0x801F4494 the same way
 *           and switches on the same mode byte with the same two arm constants;
 *           that sibling differs in the table base word and in the immediate of
 *           its own j, so its body is shape evidence and not a clone, and a
 *           scan of all 396 shipped payloads finds this 80-byte span and its
 *           table-base word 0x9C442294 exactly once.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusFromTable449cAndProgress1Or0AWorld02Area08013_801F3C94(
    void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F449C[mode];
  switch (mode) {
  case 0:
    g_ScenarioProgress = 1;
    break;
  case 1:
    g_ScenarioProgress = 0xA;
    break;
  }
}
