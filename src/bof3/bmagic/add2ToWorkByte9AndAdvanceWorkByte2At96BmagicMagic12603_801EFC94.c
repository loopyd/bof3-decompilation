#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801EFC94
 * @behavior Adds two modulo 256 to scratchpad work-object byte 9. When the
 * resulting byte equals 0x96, reloads the work pointer and increments byte 2.
 * The overlay handler table contains this entry at 0x801F1280. Takes no
 * arguments, returns nothing and touches no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByte9AndAdvanceWorkByte2At96BmagicMagic12603_801EFC94(void) {
    if ((D_1F800044[9] += 2) == 0x96) {
        D_1F800044[2]++;
    }
}
