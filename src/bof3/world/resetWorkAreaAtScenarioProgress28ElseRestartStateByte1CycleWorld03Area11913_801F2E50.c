#include "bof3/bof3.h"

/* Shared scenario progress byte of the main executable; the composed shared map
 * already binds this address for the target. */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind data */

/* Work record published at the scratchpad pointer cell 0x1F800044. */
extern u8* world03Area11913WorkRecord; /* @source 0x1F800044 @kind data */

/* Shared main-executable work-area reset helper. */
extern void func_80196070(void);

/* @source 0x801F2E50
 * @behavior Overlay step handler, entry 7 of the byte-1 handler table based at
 *           0x801F45CC: reads the shared scenario progress byte of the main
 *           executable and compares it with 0x28. While the progress stands at
 *           0x28 it calls the shared main-executable work-area reset at
 *           0x80196070 and leaves the work record alone; on every other
 *           progress value it stores 0 into byte 0x01 of the work record
 *           published at the scratchpad pointer cell 0x1F800044, which sends
 *           the next dispatch to entry 0 of the same table
 *           (clearCountdownByte9AndSeedStateByte1FromRandomModeTable45c4World03Area11913_801F2C48,
 *           0x801F2C48) so that handler reseeds that byte from the 8-entry mode
 *           table at 0x801F45C4. Entry 6
 *           (retreatWorkWordAndSetScratchStateToMode7AndClearCountdownWorld03Area11913_801F2DFC,
 *           0x801F2DFC) arms that byte with 7, so this body ends the retreat
 *           cycle it was entered from unless the progress has reached 0x28.
 *           It takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetWorkAreaAtScenarioProgress28ElseRestartStateByte1CycleWorld03Area11913_801F2E50(void) {
  if (g_ScenarioProgress == 0x28) {
    func_80196070();
  } else {
    world03Area11913WorkRecord[1] = 0;
  }
}
