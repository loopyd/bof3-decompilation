#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern void clearWorkBytesBmagicMagic12403_801F088C(void); /* @source 0x801F088C @kind function */

/* @source 0x801F0368
 * @behavior Increments work byte 0xA modulo 256, reloads the scratchpad
 *           work pointer at 0x1F800044, and subtracts four from work byte 9.
 *           When the truncated stored byte is zero, decrements byte 0xB
 *           through the shared pointer cell at 0x801EB4E0 and calls the
 *           local helper that clears work bytes 0 through 4. Takes no
 *           arguments and returns nothing. The handler address appears
 *           in the payload table at 0x801F08FC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAThenSubtract4FromByte9AndDecrementGlobalByteBAndClearWhenZeroBmagicMagic12403_801F0368(void) {
    g_battle_work[0xA]++;
    if ((g_battle_work[9] -= 4) == 0) {
        D_801EB4E0[0xB] -= 1;
        clearWorkBytesBmagicMagic12403_801F088C();
    }
}
