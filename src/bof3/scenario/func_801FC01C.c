#include "bof3/scenario/scena00_internal.h"

/**
 * @source 0x801FC01C
 * @behavior Seeds the twelve fixed-point channels of every active scenario record: each
 * 0x28-byte record's twelve s16 output channels at 0x02..0x18 are stored into the high half
 * of the matching D_801FD030 row, the record's three s16 axis words at 0x1A/0x1C/0x1E fill
 * the D_801FCA90 row left-shifted by 11 - (rand() & 3), and the twelve D_801FD5D0 offsets of
 * that row are all set to (rand() & 7) + 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801FC01C(void)
{
    u8 *records = D_80147BD8;
    s8 i;

#define SEED_CHANNEL(n, axis)                                        \
    D_801FD030[i][n] = REC_CHANNEL((n) + 1) << 16;                   \
    D_801FCA90[i][n] = REC_CHANNEL(13 + (axis)) << shift

#define REC_CHANNEL(offset) ((s16 *)(records + i * 0x28))[offset]

#define SEED_OFFSET(n) D_801FD5D0[i][n] = value

    for (i = 0; i < *D_80147BDC; i++) {
        s32 shift = 11 - (rand() & 3);
        s32 value;

        SEED_CHANNEL(0, 0);
        SEED_CHANNEL(1, 1);
        SEED_CHANNEL(2, 2);
        SEED_CHANNEL(3, 0);
        SEED_CHANNEL(4, 1);
        SEED_CHANNEL(5, 2);
        SEED_CHANNEL(6, 0);
        SEED_CHANNEL(7, 1);
        SEED_CHANNEL(8, 2);
        SEED_CHANNEL(9, 0);
        SEED_CHANNEL(10, 1);
        SEED_CHANNEL(11, 2);

        value = (rand() & 7) + 1;

        SEED_OFFSET(0);
        SEED_OFFSET(1);
        SEED_OFFSET(2);
        SEED_OFFSET(3);
        SEED_OFFSET(4);
        SEED_OFFSET(5);
        SEED_OFFSET(6);
        SEED_OFFSET(7);
        SEED_OFFSET(8);
        SEED_OFFSET(9);
        SEED_OFFSET(10);
        SEED_OFFSET(11);
    }

#undef SEED_OFFSET
#undef REC_CHANNEL
#undef SEED_CHANNEL
}
