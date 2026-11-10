#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801EFC28
 * @behavior Increments scratchpad work-object byte 9 modulo 256, reloads
 * the work pointer and increments byte 0xA modulo 256. When the resulting
 * byte 0xA equals 0x10, reloads the pointer and increments byte 2. The
 * overlay handler table names this entry at 0x801F127C. Takes no arguments
 * and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkBytes9AThenAdvanceWorkByte2WhenAIs10BmagicMagic12603_801EFC28(void) {
    D_1F800044[9]++;
    if ((D_1F800044[0xA] += 1) == 0x10) {
        D_1F800044[2]++;
    }
}
