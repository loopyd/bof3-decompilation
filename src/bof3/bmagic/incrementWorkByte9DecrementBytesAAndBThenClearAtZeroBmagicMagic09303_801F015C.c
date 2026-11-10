#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern void clearWorkBytesBmagicMagic09303_801F0620(void); /* @source 0x801F0620 */

/* @source 0x801F015C
 * @behavior Increments work byte 9 and decrements work bytes A and B,
 * reloading the scratchpad work pointer before each update. When the new
 * byte B is zero, decrements byte B of the record at pointer cell 0x801EB4E0
 * and calls the local helper that clears work bytes 0 through 4.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9DecrementBytesAAndBThenClearAtZeroBmagicMagic09303_801F015C(void) {
  g_battle_work[9]++;
  g_battle_work[0xA]--;
  if (--g_battle_work[0xB] == 0) {
    D_801EB4E0[0xB]--;
    clearWorkBytesBmagicMagic09303_801F0620();
  }
}
