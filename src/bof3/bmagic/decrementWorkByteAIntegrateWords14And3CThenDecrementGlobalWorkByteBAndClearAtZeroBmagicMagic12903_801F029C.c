#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern void clearWorkBytesBmagicMagic12903_801F0564(void); /* @source 0x801F0564 */

/* @source 0x801F029C
 * @behavior Decrements work byte 0xA modulo 256, adds work word 0x20 to
 * word 0x14, then adds the updated word 0x14 to word 0x3C. When the byte
 * reaches zero, decrements byte 0xB of the record pointed to by 0x801EB4E0
 * and clears work bytes 0 through 4. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAIntegrateWords14And3CThenDecrementGlobalWorkByteBAndClearAtZeroBmagicMagic12903_801F029C(void) {
    u8 *work = g_battle_work;
    u8 count = work[0xA] - 1;
    u32 word14 = FIELD_REF(u32, work, 0x14) + FIELD_REF(u32, work, 0x20);
    u32 word3C = FIELD_REF(u32, work, 0x3C) + word14;

    work[0xA] = count;
    FIELD_REF(u32, work, 0x14) = word14;
    FIELD_REF(u32, work, 0x3C) = word3C;
    if (count == 0) {
        D_801EB4E0[0xB]--;
        clearWorkBytesBmagicMagic12903_801F0564();
    }
}
