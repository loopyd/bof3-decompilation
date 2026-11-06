#include "bof3/bof3.h"

/* First parameter byte of the current commu request; func_801D0C68 rewrites it
 * when it publishes request mode 1. */
extern u8 D_801D4DF8; /* @source 0x801D4DF8 @kind data */

/* Shared entry-count byte in main RAM. func_801D1778 clamps the overlay's
 * selection byte against it, so it bounds how many entries this overlay lists. */
extern u8 D_801455C3; /* @source 0x801455C3 @kind data */

/* @source 0x801D4474
 * @behavior Computes the signed y origin of the commu01/07 entry list. When the
 * request parameter byte at 0x801D4DF8 is zero the list starts at 0x6A and
 * scrolls up 9 pixels per entry past the fifth; any other value starts at 0xAC
 * and moves up 12 pixels per entry counted by 0x801455C3. The handlers
 * func_801D1778, func_801D1A94, func_801D1E48 and func_801D2110 store the
 * result into the second halfword of a stack coordinate pair before drawing;
 * func_801D4178 then draws 0x801455C3 lines from the stride-8 ASCII label
 * table at 0x801D4D34 ("Scholar", "Merchant", "Inn", "Gift", and so on)
 * starting 2 pixels below this value, so the result is that list's y origin.
 * @status exact
 * @match 100.00
 * @residual none
 */
s16 computeEntryListOriginY(void) {
    s16 y;
    u8 count;

    if (D_801D4DF8 == 0) {
        count = D_801455C3;
        y = 0x6A;
        if (count >= 5) {
            y -= (count - 4) * 9;
        }
    } else {
        y = 0xAC - D_801455C3 * 12;
    }
    return y;
}
