#include "bof3/bof3.h"

extern u8 D_80146384; /* @source 0x80146384 @kind data */
extern u8 D_80146374; /* @source 0x80146374 @kind data */
extern u16 D_801462E8; /* @source 0x801462E8 @kind data */
extern u8 *D_8014638C; /* @source 0x8014638C @kind data */
extern u8 D_80145988; /* @source 0x80145988 @kind data */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_800A9820(u8 value); /* @source 0x800A9820 @kind function */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EECE0
 * @behavior When bytes 0x80146384 and 0x80146374 are both below 3 or both
 * at least 3, forwards the former to helper 0x800A9820, raises bit 4 in
 * halfword 0x801462E8 and invokes helper 0x801E5988. Otherwise copies
 * words 0x34, 0x38 and 0x3C from the record at pointer cell 0x8014638C
 * to the scratchpad work object, copies sixteen halfwords from 0x80036C00
 * to 0x8003AC00 with bit 15 set, stores 1 at byte 0x80145988, clears
 * work bytes 0xB and 9 and advances work byte 1. Reloads the work pointer
 * between the byte updates. Takes no arguments and returns nothing;
 * table word 0x801EEC04 selects this callback through work byte 1.
 * Unknown cell and helper roles remain address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteAndRaiseFlag4OrCopyPoseAnd16HalfwordsThenAdvanceWorkByte1BmagicMagic11303_801EECE0(void) {
    u8 index;
    u8 *record;
    u8 *work;
    u8 *clearB;
    u8 *clear9;
    u8 *step;

    if ((D_80146384 < 3 && D_80146374 < 3) ||
        (D_80146384 >= 3 && D_80146374 >= 3)) {
        func_800A9820(D_80146384);
        D_801462E8 |= 4;
        func_801E5988();
    } else {
        index = 0;
        record = D_8014638C;
        work = g_battle_work;
        *(u32 *)(work + 0x34) = *(u32 *)(record + 0x34);
        *(u32 *)(work + 0x38) = *(u32 *)(record + 0x38);
        *(u32 *)(work + 0x3C) = *(u32 *)(record + 0x3C);
        for (; index < 0x10; index++) {
            PSX_PTR(u16, 0x8003AC00u)[index] =
                PSX_PTR(u16, 0x80036C00u)[index] | 0x8000;
        }
        clearB = g_battle_work;
        D_80145988 = 1;
        clearB[0xB] = 0;
        clear9 = g_battle_work;
        clear9[9] = 0;
        step = g_battle_work;
        step[1]++;
    }
}
