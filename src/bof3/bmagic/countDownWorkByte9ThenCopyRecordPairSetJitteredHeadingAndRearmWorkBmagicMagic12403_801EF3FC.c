#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern u16 D_1F800008; /* @source 0x1F800008 @kind data */
extern u16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern u16 D_1F80000C; /* @source 0x1F80000C @kind data */
extern u16 D_1F80000E; /* @source 0x1F80000E @kind data */
extern void func_800AFD9C(void); /* @source 0x800AFD9C */
extern void func_800AF764(void); /* @source 0x800AF764 */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */

/* @source 0x801EF3FC
 * @behavior Decrements work byte 9 modulo 256. On zero, calls helpers
 * 0x800AFD9C and 0x800AF764, saves work halfwords 0x2E and 0x30 to
 * scratchpad halfwords 0xC and 0xE, and copies that pair from the record at
 * 0x801EB4E0 into both work and scratchpad halfwords 8 and 0xA. Stores the
 * heading of the previous-minus-current signed pair in work word 0x14,
 * then adds or subtracts (rand() & 7) << 6 according to work byte 4 bit 0.
 * Clears word 0xC, sets byte 0xB to rand() & 0x3F, rearms byte 9 to 16,
 * clears byte 0xA and advances byte 2. Entry 0 of the work-byte-2 table at
 * 0x801F08E4; takes no arguments and returns nothing. Other field roles
 * and the two no-argument helpers remain unknown.
 * @status partial
 * @match 79.80
 * @residual 79 of 99 instructions; 388 bytes versus 396. The first
 * forward branch targets the shortened epilogue. The saved scratchpad
 * halfword 0xC is forwarded rather than reloaded, and the jitter arms
 * allocate the work pointer and heading value to opposite registers.
 */
void countDownWorkByte9ThenCopyRecordPairSetJitteredHeadingAndRearmWorkBmagicMagic12403_801EF3FC(void) {
    u8 *work;
    u8 *record;
    u8 *headingWork;
    u8 *jitterWork;
    u16 y;
    u16 *previousX;
    s32 heading;
    s32 roll;

    if (--g_battle_work[9] == 0) {
        func_800AFD9C();
        func_800AF764();
        work = g_battle_work;
        record = D_801EB4E0;
        previousX = &D_1F80000C;
        *previousX = *(u16 *)(work + 0x2E);
        D_1F80000E = *(u16 *)(work + 0x30);
        *(u16 *)(work + 0x2E) = *(u16 *)(record + 0x2E);
        D_1F800008 = *(u16 *)(work + 0x2E);
        y = *(u16 *)(record + 0x30);
        *(u16 *)(work + 0x30) = y;
        D_1F80000A = y;
        heading = func_8017A138(
            *(s16 *)previousX - *(s16 *)&D_1F800008,
            *(s16 *)&D_1F80000E - (s16)y);
        headingWork = g_battle_work;
        *(s32 *)(headingWork + 0x14) = heading;
        if (headingWork[4] & 1) {
            roll = rand();
            jitterWork = g_battle_work;
            *(s32 *)(jitterWork + 0x14) += (roll & 7) << 6;
        } else {
            roll = rand();
            jitterWork = g_battle_work;
            *(s32 *)(jitterWork + 0x14) -= (roll & 7) << 6;
        }
        *(s32 *)(g_battle_work + 0xC) = 0;
        roll = rand();
        g_battle_work[0xB] = roll & 0x3F;
        g_battle_work[9] = 16;
        g_battle_work[0xA] = 0;
        g_battle_work[2]++;
    }
}
