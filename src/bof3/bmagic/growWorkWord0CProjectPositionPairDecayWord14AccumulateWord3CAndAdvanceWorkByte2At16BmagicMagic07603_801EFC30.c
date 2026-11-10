#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */
extern s32 func_801782FC(s32 angle); /* @source 0x801782FC */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 */

/* @source 0x801EFC30
 * @behavior Adds two to work word 0x0C and publishes (work byte 0xB & 15)
 * shifted left eight to scratchpad halfword 4. Samples 0x801782FC at that
 * angle and 0x801783C8 at the signed scratchpad halfword. Each sample times
 * work word 0x0C offsets shared record words 0x34 and 0x38 into work words
 * 0x34 and 0x38. When signed work word 0x14 exceeds word 0x20, subtracts
 * word 0x20 from word 0x14 and adds the remainder to word 0x3C. Adds two
 * to work byte 9 and advances byte 2 when byte 9 reaches 16. Reloads the
 * work cell after both calls and before the byte updates. Table callback
 * at 0x801EEC20; takes no arguments and returns nothing. Other roles stay
 * offset-based.
 * @status exact
 * @match 100.00
 * @residual none
 */
void growWorkWord0CProjectPositionPairDecayWord14AccumulateWord3CAndAdvanceWorkByte2At16BmagicMagic07603_801EFC30(void) {
    u8 *work;
    u8 *work2;
    u8 *work3;
    u8 *work4;
    u8 counter;
    s16 *angle;
    s32 direction;
    s32 sample;

    work = g_battle_work;
    angle = &D_1F800004;
    direction = work[0xB] & 15;
    *(s32 *)(work + 0x0C) += 2;
    *angle = (u8)direction << 8;
    sample = func_801782FC(direction << 8);
    work2 = g_battle_work;
    *(s32 *)(work2 + 0x34) = *(s32 *)(D_801EB4E0 + 0x34) + sample * *(s32 *)(work2 + 0x0C);
    sample = func_801783C8(*angle);
    work3 = g_battle_work;
    *(s32 *)(work3 + 0x38) = *(s32 *)(D_801EB4E0 + 0x38) + sample * *(s32 *)(work3 + 0x0C);
    if (*(s32 *)(work3 + 0x20) < *(s32 *)(work3 + 0x14)) {
        *(s32 *)(work3 + 0x14) -= *(s32 *)(work3 + 0x20);
        *(s32 *)(work3 + 0x3C) += *(s32 *)(work3 + 0x14);
    }
    work4 = g_battle_work;
    counter = work4[9] + 2;
    work4[9] = counter;
    if (counter == 16) {
        g_battle_work[2]++;
    }
}
