#include "bof3/bof3.h"

extern void clearWorkFlags(void); /* @source 0x80196070 @kind function */
extern u16 D_80146876;            /* @source 0x80146876 @kind data */

/* @source 0x801F2DF0
 * @behavior Overlay step handler, entry 8 and the last word of the nine-word
 *           per-frame step handler run based at 0x801F3E68 that the dispatcher
 *           0x801F2C04 indexes with byte +0x01 of the work record published at
 *           the scratchpad pointer cell 0x1F800044: runs the shared work-area
 *           reset at 0x80196070, the game executable map name clearWorkFlags,
 *           which clears the scratchpad work-area bytes 0x00 through 0x04, then
 *           stores zero into the shared halfword 0x80146876 that the scenario
 *           code steps as a countdown. It reads no state of its own, touches
 *           nothing else, takes no arguments and returns nothing; the 0x18-byte
 *           frame only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetWorkAreaAndClearCountdownWorld03Area14413_801F2DF0(void) {
  clearWorkFlags();
  D_80146876 = 0u;
}
