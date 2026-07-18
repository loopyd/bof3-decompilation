#include "internal.h"

/* @behavior Pending analysis
 * @source 0x8017212C */

extern u16 D_8018DB50[];
extern u16 D_8018DBF0[];
extern u16 D_8018DBF4[];
extern u8 D_8018DC0B[];
extern u16 D_8018E7F2;
extern u16 D_80190C58[];

void func_8017212C(void)
{
    u16 flags;
    u16 bit_a;
    u16 bit_b;
    u16 tmp;
    s32 byte_idx;
    s32 hword_idx;

    flags = D_8018E7F2;
    if (flags < 0x10U) {
        bit_b = 0;
        bit_a = 1 << flags;
    } else {
        bit_b = 1 << (flags - 0x10);
        bit_a = 0;
    }
    byte_idx = flags * 0x34;
    hword_idx = flags * 0x1A;
    D_8018DC0B[byte_idx] = 0;
    D_8018DBF4[hword_idx] = 0;
    D_8018DBF0[hword_idx] = 0;
    tmp = D_80190C58[0] | bit_a;
    D_80190C58[0] = tmp;
    D_8018DB50[0] &= ~tmp;
    tmp = D_80190C58[1] | bit_b;
    D_80190C58[1] = tmp;
    D_8018DB50[1] &= ~tmp;
}
