#include "bof3/bof3.h"

/* A byte store through the work pointer can alias its scratchpad pointer cell,
 * so the cell is reloaded before the following byte read-modify-write. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF640
 * @behavior If unsigned work byte 0xB is below five, stores 8 to work byte 9,
 * then reloads the scratchpad work pointer and increments work byte 2. Byte 2
 * selects the handler copied from the six-entry table at 0x801EEC14 by
 * dispatchWorkByte2HandlerBmagicMagic16903_801EF254; this function occupies
 * entry 3. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9To8AndIncrementWorkByte2WhenWorkByteBBelow5BmagicMagic16903_801EF640(
    void) {
  if (g_battle_work[0xB] < 5u) {
    g_battle_work[9] = 8;
    g_battle_work[2]++;
  }
}
