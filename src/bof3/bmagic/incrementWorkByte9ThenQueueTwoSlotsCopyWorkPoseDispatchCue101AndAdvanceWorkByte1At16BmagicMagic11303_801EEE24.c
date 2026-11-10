#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC338[]; /* @source 0x801EC338 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC33A[]; /* @source 0x801EC33A @kind data */
extern u8 D_801EC33B[]; /* @source 0x801EC33B @kind data */
extern u8 D_801EC364[]; /* @source 0x801EC364 @kind data */
extern u8 D_801EC368[]; /* @source 0x801EC368 @kind data */
extern u8 D_801EC36E[]; /* @source 0x801EC36E @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */
extern void func_8015DF18(u32 cue_id); /* @source 0x8015DF18 */

/* @source 0x801EEE24
 * @behavior Increments scratchpad work byte 9 modulo 256. At 16, claims two
 *           0x78-byte slots through 0x801E590C(1, 3), truncating each index to
 *           a byte without checking failure. Clears slot bytes 1, 9 and A,
 *           sets byte B to 0 then 1, publishes the work pointer in word 0x74,
 *           and sets byte 8 from (shared battle work byte 8 + 2) & 3. Reloads
 *           the work pointer for each slot and copies words 0x34 and 0x38 and
 *           halfword 0x3E to slot offsets 0x34, 0x38 and 0x3E. Dispatches cue
 *           0x101 and increments work byte 1. Takes no arguments and returns
 *           nothing. Other slot fields and helper roles remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9ThenQueueTwoSlotsCopyWorkPoseDispatchCue101AndAdvanceWorkByte1At16BmagicMagic11303_801EEE24(void) {
    u8 *work;
    u8 *slotWork;
    u8 *firstPose;
    u8 *secondPose;
    u8 *record;
    u32 index;
    u8 selector;

    work = g_battle_work;
    work[9]++;
    if (work[9] == 16) {
        index = (u8)func_801E590C(1, 3) * 0x78;
        slotWork = g_battle_work;
        D_801EC331[index] = 0;
        D_801EC33B[index] = 0;
        record = D_801EB4E0;
        *(u8 **)&D_801EC3A4[index] = slotWork;
        selector = record[8];
        D_801EC339[index] = 0;
        D_801EC33A[index] = 0;
        D_801EC338[index] = (selector + 2) & 3;
        firstPose = g_battle_work;
        *(u32 *)&D_801EC364[index] = *(u32 *)(firstPose + 0x34);
        *(u32 *)&D_801EC368[index] = *(u32 *)(firstPose + 0x38);
        *(u16 *)&D_801EC36E[index] = *(u16 *)(firstPose + 0x3E);
        index = (u8)func_801E590C(1, 3) * 0x78;
        slotWork = g_battle_work;
        D_801EC331[index] = 0;
        D_801EC33B[index] = 1;
        record = D_801EB4E0;
        *(u8 **)&D_801EC3A4[index] = slotWork;
        selector = record[8];
        D_801EC339[index] = 0;
        D_801EC33A[index] = 0;
        D_801EC338[index] = (selector + 2) & 3;
        secondPose = g_battle_work;
        *(u32 *)&D_801EC364[index] = *(u32 *)(secondPose + 0x34);
        *(u32 *)&D_801EC368[index] = *(u32 *)(secondPose + 0x38);
        *(u16 *)&D_801EC36E[index] = *(u16 *)(secondPose + 0x3E);
        func_8015DF18(0x101);
        g_battle_work[1]++;
    }
}
