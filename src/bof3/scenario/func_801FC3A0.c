#include "bof3/scenario/scena00_internal.h"

/**
 * @source 0x801FC3A0
 * @behavior Advances twelve fixed-point channels for each active scenario record and subtracts their per-channel offsets from a companion record.
 */
void func_801FC3A0(void)
{
    s8 i;
    s32 value;
    s16* output;
    u16* companion;
    u8* outputs = D_80147BD8;
    u8* companions = D_80147AA8;

#define UPDATE_CHANNEL(n) \
    value = D_801FD030[i][n] + D_801FCA90[i][n]; \
    D_801FD030[i][n] = value; \
    output[(n) + 1] = value >> 16
#define SUBTRACT_CHANNEL(n) \
    companion[(n) + 1] -= D_801FD5D0[i][n]

    for (i = 0; i < *D_80147BDC; i++) {
        output = (s16*)(outputs + i * 0x28);
        UPDATE_CHANNEL(0);
        UPDATE_CHANNEL(1);
        UPDATE_CHANNEL(2);
        UPDATE_CHANNEL(3);
        UPDATE_CHANNEL(4);
        UPDATE_CHANNEL(5);
        UPDATE_CHANNEL(6);
        UPDATE_CHANNEL(7);
        UPDATE_CHANNEL(8);
        UPDATE_CHANNEL(9);
        UPDATE_CHANNEL(10);
        companion = (u16*)(companions + i * 0x28);
        UPDATE_CHANNEL(11);
        SUBTRACT_CHANNEL(0);
        SUBTRACT_CHANNEL(1);
        SUBTRACT_CHANNEL(2);
        SUBTRACT_CHANNEL(3);
        SUBTRACT_CHANNEL(4);
        SUBTRACT_CHANNEL(5);
        SUBTRACT_CHANNEL(6);
        SUBTRACT_CHANNEL(7);
        SUBTRACT_CHANNEL(8);
        SUBTRACT_CHANNEL(9);
        SUBTRACT_CHANNEL(10);
        SUBTRACT_CHANNEL(11);
    }

#undef SUBTRACT_CHANNEL
#undef UPDATE_CHANNEL
}
