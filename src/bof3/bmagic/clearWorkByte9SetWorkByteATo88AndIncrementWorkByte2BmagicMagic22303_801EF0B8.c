#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF0B8
 * @behavior Clears work-record byte 9, sets byte 0xA to 0x88, and
 * increments byte 2 modulo 256. Reloads the scratchpad work pointer at
 * 0x1F800044 before each operation. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte9SetWorkByteATo88AndIncrementWorkByte2BmagicMagic22303_801EF0B8(void) {
  g_battle_work[9] = 0;
  g_battle_work[0xA] = 0x88;
  g_battle_work[2]++;
}
