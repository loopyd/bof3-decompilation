#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF094
 * @behavior Writes work byte 9 times four plus 0xC0 to scratchpad halfword
 *           zero, increments work byte 0xA, reloads the work pointer and
 *           increments byte 9. When the incremented byte 9 equals 0x28,
 *           decrements byte 0xB of the record at pointer cell 0x801EB4E0
 *           and calls the no-argument helper at 0x801E5988.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScratchHalfword0FromWorkByte9Times4PlusC0IncrementBytesA9ThenDecrementGlobalByteBAndCall5988At28BmagicMagic10303_801EF094(void) {
    u8 *work;
    u8 counter;

    work = g_battle_work;
    SPAD_REF(u16, 0u) = work[9] * 4 + 0xC0;
    work[0xA]++;
    work = g_battle_work;
    counter = work[9] + 1;
    work[9] = counter;
    if (counter == 0x28) {
        D_801EB4E0[0xB]--;
        func_801E5988();
    }
}
