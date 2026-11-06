#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original materialises the cell
 * address (lui $vN,0x1F80 / lw $vN,68($vN)) once for the increment's load and
 * store pair and again for the equality test, whose load the byte-2 arm then
 * shares, so the cell is a plain pointer read once per access group. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF38C
 * @behavior Adds two to scratchpad work-object byte 0xA modulo 256 with a
 * byte-width add (lbu / addiu 2 / sb), then re-reads the cell and that byte
 * and, when it equals 0x10, increments the work object's byte 0x2 through the
 * same reload, advancing to the next byte-0x2 handler. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByteAAndAdvanceWorkByte2At10BmagicMagic12903_801EF38C(void) {
    g_battle_work[0xA] += 2;
    if (g_battle_work[0xA] == 0x10) {
        g_battle_work[2]++;
    }
}
