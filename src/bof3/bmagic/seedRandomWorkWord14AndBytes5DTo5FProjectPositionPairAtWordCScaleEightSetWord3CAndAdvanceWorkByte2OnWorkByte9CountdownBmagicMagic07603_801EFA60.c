#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */
extern s32 func_801782FC(s32 angle); /* @source 0x801782FC */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */
extern int rand(void);

/* @source 0x801EFA60
 * @behavior Decrement work byte 9 and return while that countdown is
 * nonzero. At zero, request main-executable helper 0x8015D71C with code
 * 0x101 when work byte 0xB is clear, set work word 0x0C to 8 and publish
 * (work byte 0xB & 15) shifted left eight to scratchpad halfword 4.
 * Sample 0x801782FC at that angle and 0x801783C8 at the signed scratchpad
 * halfword. Each sample times work word 0x0C offsets shared record words
 * 0x34 and 0x38 into work words 0x34 and 0x38, and shared record word 0x3C
 * biased by 0x800000 replaces work word 0x3C. Seeds work word 0x14 with a
 * masked rand() value in [0x200000, 0x300000) and work word 0x20 with that
 * value divided by 16, seeds work bytes 0x5D through 0x5F with masked
 * rand() values plus 6, clears work byte 9, sets work byte 0xA to 0x10 and
 * advances work byte 2. Reloads the work cell after every call and before
 * each store group. Table callback at 0x801EEC1C; takes no arguments and
 * returns nothing. Other roles stay offset-based.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedRandomWorkWord14AndBytes5DTo5FProjectPositionPairAtWordCScaleEightSetWord3CAndAdvanceWorkByte2OnWorkByte9CountdownBmagicMagic07603_801EFA60(void) {
    u8 *work;
    u8 *work2;
    u8 *work3;
    u8 *work4;
    s16 *angle;
    s32 direction;
    s32 sample;
    s32 speed;

    work = g_battle_work;
    if (--work[9] != 0) {
        return;
    }
    if (g_battle_work[0xB] == 0) {
        func_8015D71C(0x101);
    }
    work2 = g_battle_work;
    angle = &D_1F800004;
    direction = work2[0xB] & 15;
    *(s32 *)(work2 + 0x0C) = 8;
    *angle = (u8)direction << 8;
    sample = func_801782FC(direction << 8);
    work3 = g_battle_work;
    *(s32 *)(work3 + 0x34) = *(s32 *)(D_801EB4E0 + 0x34) + sample * *(s32 *)(work3 + 0x0C);
    sample = func_801783C8(*angle);
    work4 = g_battle_work;
    *(s32 *)(work4 + 0x38) = *(s32 *)(D_801EB4E0 + 0x38) + sample * *(s32 *)(work4 + 0x0C);
    *(s32 *)(work4 + 0x3C) = *(s32 *)(D_801EB4E0 + 0x3C) + 0x800000;
    speed = (rand() & 3) << 20;
    speed += 0x200000;
    *(s32 *)(g_battle_work + 0x14) = speed;
    *(s32 *)(g_battle_work + 0x20) = speed / 16;
    g_battle_work[0x5D] = (rand() & 7) + 6;
    g_battle_work[0x5E] = (rand() & 7) + 6;
    g_battle_work[0x5F] = (rand() & 7) + 6;
    g_battle_work[9] = 0;
    g_battle_work[0xA] = 0x10;
    g_battle_work[2]++;
}
