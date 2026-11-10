#include "bof3/bof3.h"

extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern void func_801E5988(void); /* @source 0x801E5988 */

/* @source 0x801EFCE4
 * @behavior Increments scratchpad work-object byte 9 modulo 256, reloads
 * the work pointer and decrements byte 0xA modulo 256. When the resulting
 * byte 0xA is zero, decrements byte 0xB of the shared record reached through
 * 0x801EB4E0 and calls the helper at 0x801E5988. The handler table names
 * this entry at 0x801F1284. Takes no arguments and returns nothing.
 * The original 120-byte span compared equal to magic145/03 at 0x801EF0A0;
 * that source supplied the statement order and pointer-cell spelling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic12603_801EFCE4(void)
{
    u8 **slots;

    slots = SPAD_PTR_TABLE(u8);

    slots[0x11][0x09] += 1;
    if (--slots[0x11][0x0A] == 0) {
        D_801EB4E0[0x0B] -= 1;
        func_801E5988();
    }
}
