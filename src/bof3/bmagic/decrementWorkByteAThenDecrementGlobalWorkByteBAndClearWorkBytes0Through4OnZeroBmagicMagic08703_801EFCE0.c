#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* @source 0x801EFCE0
 * @behavior Decrements byte A of the scratchpad work record modulo 256.
 * When the resulting byte is zero, decrements byte B of the record at the
 * shared pointer cell 0x801EB4E0, then clears work bytes 0 through 4 in order.
 * Reloads the scratchpad pointer before each clear because the preceding
 * byte store may alias its cell. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAThenDecrementGlobalWorkByteBAndClearWorkBytes0Through4OnZeroBmagicMagic08703_801EFCE0(void) {
  if (--g_battle_work[0xA] == 0) {
    D_801EB4E0[0xB]--;
    g_battle_work[0] = 0;
    g_battle_work[1] = 0;
    g_battle_work[2] = 0;
    g_battle_work[3] = 0;
    g_battle_work[4] = 0;
  }
}
