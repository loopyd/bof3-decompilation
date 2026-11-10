#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern s32 D_1F800000; /* @source 0x1F800000 @kind data */
extern s32 D_1F800004; /* @source 0x1F800004 @kind data */
extern s32 func_801783C8(s32 angle);
extern s32 func_801782FC(s32 angle);
extern void func_801E5988(void);

/* @source 0x801F0314
 * @behavior Increment work byte B and word 0x0C, publish the new word masked
 *           with 0x7F and shifted left five at scratchpad word 4, and publish
 *           radius 24 at scratchpad word 0. Sample 0x801783C8 and 0x801782FC
 *           at that angle and add each sample times the radius to the source
 *           record words 0x34 and 0x38, storing the pair in the work record.
 *           Subtract two from work byte A modulo 256, then re-read byte A.
 *           When zero, increment source record byte B and call 0x801E5988,
 *           whose role is unknown. The entry is published at table word
 *           0x801F09CC, takes no arguments and returns nothing.
 *           Record field roles remain offset-based.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteBAndWord0CProjectPositionPairAtRadius24ThenSubtractTwoFromByteAAndAdvanceSourceByteBAndCallE5988WhenZeroBmagicMagic09903_801F0314(void) {
    u8 *work;
    u8 *tail;
    u8 *test;
    s32 *angle;
    s32 turn;
    s32 sample;
    u32 position;
    s32 step;
    u8 countdown;

    g_battle_work[0xB]++;
    work = g_battle_work;
    turn = FIELD_REF(s32, work, 0xC) + 1;
    angle = &D_1F800004;
    FIELD_REF(s32, work, 0xC) = turn;
    *angle = (turn & 0x7F) << 5;
    D_1F800000 = 0x18;
    sample = func_801783C8(*angle);
    FIELD_REF(u32, g_battle_work, 0x34) =
        FIELD_REF(u32, D_801EB4E0, 0x34) + sample * D_1F800000;
    sample = func_801782FC(*angle);
    step = sample * D_1F800000;
    tail = g_battle_work;
    position = FIELD_REF(u32, D_801EB4E0, 0x38);
    countdown = tail[0xA] - 2;
    tail[0xA] = countdown;
    test = g_battle_work;
    FIELD_REF(u32, tail, 0x38) = position + step;
    if (test[0xA] == 0) {
        D_801EB4E0[0xB]++;
        func_801E5988();
    }
}
