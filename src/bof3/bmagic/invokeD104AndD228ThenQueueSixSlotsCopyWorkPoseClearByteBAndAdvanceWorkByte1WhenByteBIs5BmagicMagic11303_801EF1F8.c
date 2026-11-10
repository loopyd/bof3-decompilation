#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC334[]; /* @source 0x801EC334 @kind data */
extern u8 D_801EC338[]; /* @source 0x801EC338 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC33A[]; /* @source 0x801EC33A @kind data */
extern u8 D_801EC33B[]; /* @source 0x801EC33B @kind data */
extern u8 D_801EC364[]; /* @source 0x801EC364 @kind data */
extern u8 D_801EC368[]; /* @source 0x801EC368 @kind data */
extern u8 D_801EC36E[]; /* @source 0x801EC36E @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */
extern void func_8015D104(u32 arg0, u32 arg1); /* @source 0x8015D104 */
extern void func_8015D228(u32 arg0); /* @source 0x8015D228 */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* @source 0x801EF1F8
 * @behavior Calls 0x8015D104(0x14, 0x10) and 0x8015D228(0x14). When
 *           scratchpad work byte B is 5, claims six 0x78-byte slots through
 *           0x801E590C(1, 3), truncating each index to a byte without a
 *           failure check. Sets slot byte 1 to 3, byte 4 to the ordinal's
 *           low bit, byte B to ordinal / 2, byte 9 to (ordinal / 2) * 8 + 1,
 *           byte A to zero and byte 8 to (shared battle work byte 8 + 2) & 3.
 *           Publishes the work pointer in slot word 0x74 and copies work
 *           words 0x34 and 0x38 and halfword 0x3E into each slot. Clears
 *           byte B through the last pose pointer, reloads the work pointer
 *           and advances byte 1. Takes no arguments and returns nothing.
 *           The first two helper roles and other slot fields remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeD104AndD228ThenQueueSixSlotsCopyWorkPoseClearByteBAndAdvanceWorkByte1WhenByteBIs5BmagicMagic11303_801EF1F8(void) {
    u8 i;
    u8 selector;
    u8 *slotWork;
    u8 *record;
    u8 *pose;
    u32 index;

    func_8015D104(0x14, 0x10);
    func_8015D228(0x14);
    if (g_battle_work[0xB] == 5) {
        for (i = 0; i < 6; i++) {
            index = (u8)func_801E590C(1, 3) * 0x78;
            slotWork = g_battle_work;
            D_801EC331[index] = 3;
            D_801EC334[index] = i & 1;
            *(u8 **)&D_801EC3A4[index] = slotWork;
            record = D_801EB4E0;
            selector = record[8];
            D_801EC339[index] = (i >> 1) * 8 + 1;
            D_801EC33A[index] = 0;
            D_801EC33B[index] = i >> 1;
            D_801EC338[index] = (selector + 2) & 3;
            pose = g_battle_work;
            *(u32 *)&D_801EC364[index] = *(u32 *)(pose + 0x34);
            *(u32 *)&D_801EC368[index] = *(u32 *)(pose + 0x38);
            *(u16 *)&D_801EC36E[index] = *(u16 *)(pose + 0x3E);
        }
        pose[0xB] = 0;
        g_battle_work[1]++;
    }
}
