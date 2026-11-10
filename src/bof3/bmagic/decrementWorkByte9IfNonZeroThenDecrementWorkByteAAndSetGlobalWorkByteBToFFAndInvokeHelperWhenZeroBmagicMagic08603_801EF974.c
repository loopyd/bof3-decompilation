#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF974
 * @behavior Decrement scratchpad work byte 9 only when nonzero, reload the
 * work pointer, then decrement byte 0xA modulo 256. When the truncated byte-A
 * result is zero, set byte 0xB of the record pointed to by 0x801EB4E0 to
 * 0xFF and invoke the shared helper at 0x801E5988. Takes no arguments and
 * returns nothing. The pointer word at 0x801F0380 references this handler.
 * Byte stores can alias the work pointer cell, so the second access group
 * reloads it. Statement shape follows the magic121/03 handler at 0x801EF3A8;
 * original payload comparison differs only in four byte-access offsets.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9IfNonZeroThenDecrementWorkByteAAndSetGlobalWorkByteBToFFAndInvokeHelperWhenZeroBmagicMagic08603_801EF974(void) {
    if (g_battle_work[0x09] != 0) {
        g_battle_work[0x09]--;
    }
    if (--g_battle_work[0x0A] == 0) {
        D_801EB4E0[0x0B] = 0xFF;
        func_801E5988();
    }
}
