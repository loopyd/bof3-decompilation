#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF1A0
 * @behavior Adds two to scratchpad work-object byte 4 modulo 256. If the
 *           new byte is 0x10, reloads the work pointer and increments byte 2
 *           modulo 256. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByte4AndAdvanceWorkByte2At10BmagicMagic01303_801EF1A0(void) {
    if ((g_battle_work[4] += 2) == 0x10) {
        g_battle_work[2]++;
    }
}
