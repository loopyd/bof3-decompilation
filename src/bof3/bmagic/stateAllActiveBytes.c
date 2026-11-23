#include "bof3/bof3.h"

extern u8 T_801EF2A8[128][0x3C]; /* @source 0x801EF2A8 @kind data */

/* @source 0x801EEFF4
 * @behavior Clears the active byte of every record in this overlay's
 *           128-record pool at 0x801EF2A8, whose records have a 0x3C-byte
 *           stride. Records are marked active by 0x801EF020 setting byte 0 to 1
 *           and are taken as free by 0x801EEFB8 while byte 0 reads 0, so this
 *           body returns the whole pool to the free state without touching any
 *           other record byte. It takes no arguments and returns nothing: the
 *           pointer walks the pool from its base word and the tail reloads only
 *           $ra. Its only caller is the step at 0x801EEC80, which reaches it
 *           through the jal at 0x801EECD4 as the first code pointer run of this
 *           target is set up; no table word names it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateAllActiveBytes(void) {
    u8 *record;
    u8 index;

    record = T_801EF2A8[0];
    for (index = 0; index < 128; index++, record += 0x3C) {
        record[0] = 0;
    }
}
