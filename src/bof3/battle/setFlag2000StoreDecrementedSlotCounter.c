#include "bof3/battle/battle15_internal.h"

/* @source 0x800A0A40
 * @behavior Arms flag 0x2000 on D_801462E8, then stores one less than the current battler's slot counter into the active record halfword +0x04 and resets that slot counter to 1; battler indexes below 3 use the 0x140-stride table at 0x80145F18, higher indexes use the 0x118-stride table at 0x801EB6C4 rebased by -3.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setFlag2000StoreDecrementedSlotCounter(void)
{
    volatile u16* flags;
    u32 index;
    u32 offset;
    u32 slot;

    flags = &D_801462E8;
    index = *(u8*)&D_80146374;
    *flags |= 0x2000;
    if (index < 3) {
        offset = index * 0x140;
        ((volatile s16*)D_801463A0)[2] = *((u16*)(D_80145F18 + offset)) - 1;
        *((u16*)&D_80145F18[D_80146374 * 0x140]) = 1;
    } else {
        offset = (index - 3) * 0x118;
        ((volatile s16*)D_801463A0)[2] = *((u16*)(D_801EB6C4 + offset)) - 1;
        slot = D_80146374 - 3;
        *((u16*)(D_801EB6C4 + slot * 0x118)) = 1;
    }
}
