#include "internal.h"

void func_801E6A54(void)
{
    u8 value;

    value = SPAD_PTR_SLOT(u8, 0x44)[9];
    SPAD_REF(u8, 2) = value;
    SPAD_REF(u8, 1) = value;
    SPAD_REF(u8, 0) = value;
    func_801D99AC(0, 0, 0xB);
}
