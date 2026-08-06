#include "internal.h"

/* @behavior queries the current battle selection and applies input_mask when
 * a selectable target remains.
 * @source 0x800A4238
 */
u8 querySelectionApplyInput(s32 input_mask)
{
    if (func_800A3A10(D_80146374, D_80146394) == 0) {
        func_800A31E0(D_80146394, (u16)input_mask);
        return 0;
    }
    return 1;
}
