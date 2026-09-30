#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F72A0
 * @behavior Takes no arguments and returns nothing: seeds the scratchpad work
 * object's sprite through func_801F7134(1), then brightens that object's three
 * color channels (bytes 0x5D/0x5E/0x5F, the r0/g0/b0 the seeder copies into
 * the SPRT) by 2 each, and once the channel at 0x5D reads exactly 0x80 commits
 * the fade by writing 0x14 into the scenario progress byte
 * g_ScenarioProgress, setting the object's state byte at 0x01 to 2 and its
 * byte at 0x09 to 0xFF.
 * @status exact
 * @match 100.00
 * @residual none
 */
void brightenWorkObjectColorAndCommitProgress(void) {
  func_801F7134(1);
  D_1F800044[0x5D] += 2;
  D_1F800044[0x5E] += 2;
  D_1F800044[0x5F] += 2;

  if (D_1F800044[0x5D] == 0x80) {
    *(u8 *)&g_ScenarioProgress = 0x14u;
    D_1F800044[1] = 2;
    D_1F800044[9] = 0xFF;
  }
}
