#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern u8 D_80146384; /* @source 0x80146384 @kind data */
extern void func_800AFC98(s32 arg0, s32 arg1); /* @source 0x800AFC98 @kind function */
extern void func_8015D71C(s32 selector); /* @source 0x8015D71C @kind function */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 @kind function */
extern void func_800A9820(u8 index); /* @source 0x800A9820 @kind function */

/* @source 0x801EFD28
 * @behavior Adds scratchpad work word 0x20 into word 0x14 and accumulates
 * the result into halfword 0x3E. Increments byte 0xA while below 8, then
 * decrements byte 9 modulo 256 through a fresh work-pointer read. On zero,
 * calls 0x800AFC98 with (2, 0) and 0x8015D71C with 0x203, sets work words
 * 0x14 and 0x20 to 0x40 and -8, and stores in word 0xC the heading returned
 * by 0x8017A138 for the differences between the 0x801EB4E0 record and work
 * words 0x34 and 0x38. Forwards byte 0x80146384 to 0x800A9820, rearms
 * work byte 9 to 0x10 and advances handler byte 2. The payload holds this
 * callback's address at 0x801EEC50. Takes no arguments and returns nothing.
 * Other record roles and helper effects remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void integrateWorkWord14GrowByteATo8ThenOnByte9ZeroSeedMotionSetHeadingToward801EB4E0ForwardBattleByteAndAdvanceWorkByte2BmagicMagic00100_801EFD28(void) {
    u8 *work;
    u8 count;
    u8 *motion;
    u8 *record;
    s32 heading;

    work = D_1F800044;
    *(u32 *)(work + 0x14) += *(u32 *)(work + 0x20);
    count = work[0xA];
    *(u16 *)(work + 0x3E) += *(u32 *)(work + 0x14);
    if (count < 8) {
        work[0xA] = count + 1;
    }
    if (--D_1F800044[9] == 0) {
        func_800AFC98(2, 0);
        func_8015D71C(0x203);
        motion = D_1F800044;
        record = D_801EB4E0;
        *(s32 *)(motion + 0x14) = 0x40;
        *(s32 *)(motion + 0x20) = -8;
        heading = func_8017A138(
            *(s32 *)(record + 0x34) - *(s32 *)(motion + 0x34),
            *(s32 *)(record + 0x38) - *(s32 *)(motion + 0x38));
        *(s32 *)(D_1F800044 + 0xC) = heading;
        func_800A9820(D_80146384);
        D_1F800044[9] = 0x10;
        D_1F800044[2]++;
    }
}
