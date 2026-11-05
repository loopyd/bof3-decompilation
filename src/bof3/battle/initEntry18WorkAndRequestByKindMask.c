#include "bof3/battle/battle03_internal.h"

s32 func_8014DD3C(u16 entry);
void func_8014D6B8(u8 request);

/* @source 0x801E6B28
 * @behavior Loads entry 0x18 through the resident helper, sets scratch byte
 * +0x24 to 0x80 and clears byte +0x2A. Requests 0 when the current kind's
 * mask has bit 0x200, or 2 otherwise, then advances scratch byte +1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initEntry18WorkAndRequestByKindMask(void)
{
    u16 index;

    func_8014DD3C(0x18);
    ((u8*)D_1F800044)[0x24] = 0x80;
    D_1F800044->unk_2a = 0;
    index = FIELD_REF(u16, BATTLE_GLOBAL_PTR_6380, 2);
    if (D_801CA71C[index].mask_00 & 0x200) {
        func_8014D6B8(0);
    } else {
        func_8014D6B8(2);
    }
    D_1F800044->unk_01++;
}
