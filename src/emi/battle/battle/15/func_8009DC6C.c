#include "internal.h"

void func_8009DC6C(void) {
    s16 half;

    half = func_801DC044(D_80146374, D_80146394, 0xFFFF) / 2;
    ((volatile u16*)D_801463A0)[2] = half;
    if (half != 0) {
        func_800A4238(0x20);
    }
}
