#include "bof3/bof3.h"

/* Shared work record pointer cell. @source 0x80146250 @kind unknown */
extern u8* D_80146250;
/* Shared scenario-progress cell; this handler touches only its low byte.
 * @source 0x80146864 @kind unknown
 */
extern u32 g_ScenarioProgress;

/* @source 0x801F5778
 * @behavior Overlay handler: reads the +0x79 byte of the work record published
 *           through the global pointer cell 0x80146250 and, while that byte is
 *           zero, advances the low byte of the shared scenario-progress cell
 *           0x80146864 by one. The cell address is computed once and the read
 *           and the write both go through it. Takes no arguments, returns
 *           nothing and uses no stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceScenarioProgressWhenWorkByte79ClearWorld02Area10313_801F5778(void) {
  if (D_80146250[0x79] == 0) {
    *(u8 *)&g_ScenarioProgress += 1;
  }
}
