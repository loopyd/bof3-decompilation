#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern void func_801E5988(void); /* @source 0x801E5988 */

/* @source 0x801EF8E4
 * @behavior Increments scratchpad work-object byte 0xA modulo 256. When the
 * stored byte equals 12, decrements byte 0xB of the shared record reached
 * through 0x801EB4E0 and calls the helper at 0x801E5988. Takes no arguments
 * and returns nothing. The handler table word at 0x801F0BB4 names this entry.
 * The original 96-byte span compared equal to the span at 0x801EFD40 in
 * this payload. The work-pointer and byte-value spelling follows the
 * adjacent target-owned increment handler.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAThenDecrementGlobalWorkByteBAndInvokeHelperAt12BmagicMagic11303_801EF8E4(void) {
    u8 *work;
    u8 value;

    work = g_battle_work;
    value = work[0xA] + 1;
    work[0xA] = value;
    if (value == 12) {
        D_801EB4E0[0xB]--;
        func_801E5988();
    }
}
