#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_8014637C; /* @source 0x8014637C @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */
extern void func_8014D6B8(u32 flag); /* @source 0x8014D6B8 */
extern int rand(void);

/* @source 0x801EFEE8
 * @behavior Decrements work byte 9 and initializes the work record at zero.
 * Copies byte 8 from the record at 0x8014637C, seeds words C and 10 with
 * 0x10000 and zero, and calls 0x801DD350. Sets position words 34 and 38
 * to the source position plus those work words, and word 3C to the source
 * word plus 0x1000000. Stores the result of 0x8017A138 toward the record
 * at 0x801EB4E0 in word 14, then adds or subtracts rand() & 0xFF according
 * to bit 0 of byte B. Seeds the header bytes and scale words, clears bytes
 * 24 and 5C-5F, copies byte 8 bit 0 to byte 2A, clears halfword 2C and
 * calls 0x8014D6B8 with zero. Sets byte 9 to 0x10, word C to 12 and
 * advances byte 2. Entry zero of the handler table at 0x801F03B0; takes
 * no arguments and returns nothing. Unknown field roles remain offset-based.
 * @status partial
 * @match 94.00
 * @residual Nine instructions exchange the pointer and result registers in the random-offset arms.
 */
void countDownWorkByte9ThenSeedPositionHeadingAndHeaderAndAdvanceWorkByte2BmagicMagic22503_801EFEE8(void) {
    u8 *source;
    u8 *setupWork;
    u8 *motionWork;
    u8 *parityWork;
    u8 *clearWork;
    u8 *target;
    u8 *headingWork;
    u8 *randomWork;
    u8 *scaleWork;
    u8 *kindWork;
    u8 *countWork;
    u8 *stepWork;
    s32 heading;
    s32 randomValue;

    if (--g_battle_work[9] == 0) {
        source = D_8014637C;
        g_battle_work[8] = source[8];
        setupWork = g_battle_work;
        *(u32 *)(setupWork + 0xC) = 0x10000;
        *(u32 *)(setupWork + 0x10) = 0;
        func_801DD350(setupWork);
        motionWork = g_battle_work;
        target = D_801EB4E0;
        *(u32 *)(motionWork + 0x34) = *(u32 *)(source + 0x34) + *(u32 *)(motionWork + 0xC);
        *(u32 *)(motionWork + 0x38) = *(u32 *)(source + 0x38) + *(u32 *)(motionWork + 0x10);
        *(u32 *)(motionWork + 0x3C) = *(u32 *)(source + 0x3C) + 0x1000000;
        heading = func_8017A138(*(s32 *)(target + 0x34) - *(s32 *)(motionWork + 0x34),
                               *(s32 *)(target + 0x38) - *(s32 *)(motionWork + 0x38));
        headingWork = g_battle_work;
        *(s32 *)(headingWork + 0x14) = heading;
        if (headingWork[0xB] & 1) {
            randomValue = rand();
            randomWork = g_battle_work;
            *(u32 *)(randomWork + 0x14) += randomValue & 0xFF;
        } else {
            randomValue = rand();
            randomWork = g_battle_work;
            *(u32 *)(randomWork + 0x14) -= randomValue & 0xFF;
        }
        g_battle_work[0x25] = 0x1D;
        g_battle_work[0x26] = 0x80;
        scaleWork = g_battle_work;
        scaleWork[0x48] = 2;
        kindWork = g_battle_work;
        *(u32 *)(scaleWork + 0x40) = 0x10000;
        *(u32 *)(scaleWork + 0x44) = 0x10000;
        kindWork[0x27] = 2;
        g_battle_work[0x28] = 1;
        g_battle_work[0x24] = 0;
        g_battle_work[0x5C] = 0;
        g_battle_work[0x5D] = 0;
        g_battle_work[0x5E] = 0;
        g_battle_work[0x5F] = 0;
        parityWork = g_battle_work;
        parityWork[0x2A] = parityWork[8] & 1;
        g_battle_work[0x29] = 2;
        clearWork = g_battle_work;
        *(u16 *)(clearWork + 0x2C) = 0;
        clearWork[0x2B] = 1;
        func_8014D6B8(0);
        countWork = g_battle_work;
        countWork[9] = 0x10;
        stepWork = g_battle_work;
        *(u32 *)(countWork + 0xC) = 12;
        stepWork[2]++;
    }
}
