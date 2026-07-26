#include "internal.h"

/* @behavior resets the selection state, then applies input_mask when selection remains.
 * @source 0x800A41D8
 */
u8 func_800A41D8(s32 input_mask)
{
    func_800A3F28();
    if (func_800A3A10(D_80146374, D_80146394) == 0) {
        func_800A31E0(D_80146394, (u16)input_mask);
        return 0;
    }
    return 1;
}
