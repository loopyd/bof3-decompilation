#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F053C
 * @behavior Adds four to scratchpad work-object byte 9 modulo 256. The
 * byte-9 update is stored through the first work-pointer load, then the
 * pointer cell is read again and the stored byte is re-read; when that
 * value is 0x40, the same reloaded pointer increments work byte 2 modulo
 * 256. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add4ToWorkByte9AndAdvanceWorkByte2At40BmagicMagic13103_801F053C(void) {
    g_battle_work[9] += 4;
    if (g_battle_work[9] == 0x40) {
        g_battle_work[2]++;
    }
}
