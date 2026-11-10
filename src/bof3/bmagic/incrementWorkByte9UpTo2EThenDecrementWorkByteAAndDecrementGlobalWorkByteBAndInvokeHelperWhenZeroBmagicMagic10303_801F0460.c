#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801F0460
 * @behavior Increments work byte 9 unless it already equals 0x2E, then
 *           re-reads the work pointer and decrements byte 0xA. When that
 *           countdown reaches zero, decrements byte 0xB of the record at
 *           pointer cell 0x801EB4E0 and calls the helper at 0x801E5988.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9UpTo2EThenDecrementWorkByteAAndDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic10303_801F0460(void) {
    u8 *work;
    u8 counter;

    work = g_battle_work;
    counter = work[0x09];
    if (counter != 0x2E) {
        work[0x09] = counter + 1;
    }
    if (--g_battle_work[0x0A] == 0) {
        D_801EB4E0[0x0B] -= 1;
        func_801E5988();
    }
}
