#include "bof3/bof3.h"

/* The scratchpad work-object pointer cell is read again before each byte
 * store, so its address is re-materialized for each access. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F088C
 * @behavior Clears work-record bytes 0x00 through 0x04, reloading the
 * scratchpad pointer cell at 0x1F800044 before each store. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkBytesBmagicMagic12403_801F088C(void) {
  g_battle_work[0] = 0;
  g_battle_work[1] = 0;
  g_battle_work[2] = 0;
  g_battle_work[3] = 0;
  g_battle_work[4] = 0;
}
