#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF2F8
 * @behavior Handler at index 3 of the work-byte-2 table at 0x801F08F0.
 *           Increments work byte 0xA, reloads the scratchpad work pointer,
 *           and increments byte 0x9 modulo 256. When the value just stored
 *           equals 0x18, reloads the work pointer and copies its byte 0xB
 *           to byte 0xB of the record published at 0x801EB4E0, then calls
 *           the helper at 0x801E5988. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkBytesAAnd9ThenCopyByteBToGlobalWorkAndInvokeHelperAt18BmagicMagic10403_801EF2F8(void)
{
    g_battle_work[0xA]++;
    if (++g_battle_work[9] == 0x18) {
        D_801EB4E0[0xB] = g_battle_work[0xB];
        func_801E5988();
    }
}
