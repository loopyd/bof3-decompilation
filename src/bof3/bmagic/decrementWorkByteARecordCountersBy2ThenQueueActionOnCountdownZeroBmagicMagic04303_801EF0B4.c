#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

typedef struct BmagicMagic04303CounterRecord {
    u8 counter_00;
    u8 counter_01;
    u8 counter_02;
    u8 unk_03[9];
} BmagicMagic04303CounterRecord;

extern BmagicMagic04303CounterRecord D_80145BD6[]; /* @source 0x80145BD6 @kind table */
extern u8 D_80146384[]; /* @source 0x80146384 @kind unknown */
extern u16 D_801463C0; /* @source 0x801463C0 @kind unknown */
extern u8 D_801EC334[]; /* @source 0x801EC334 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC33A[]; /* @source 0x801EC33A @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

extern void func_80196670(u8 value); /* @source 0x80196670 */
extern void func_800AF814(u8 value); /* @source 0x800AF814 */
extern u32 func_800AED5C(u32 first, u32 second); /* @source 0x800AED5C */
extern u32 func_801E590C(u32 kind, u32 mode); /* @source 0x801E590C */

/* @source 0x801EF0B4
 * @behavior Subtract two from each leading byte of the stride-12 record selected
 *           by work byte 0xA, then decrement work byte 9. On zero, if work byte
 *           0xB is below two, call 0x80196670 with byte 0xA and 0x800AF814 with
 *           byte 0x80146384. When halfword 0x801463C0 is 0x2B, allocate a
 *           stride-0x78 action with arguments 1 and 0x48, set its byte 4 to zero
 *           or eight according to helper 0x800AED5C(1, byte 0x80146384), set
 *           bytes 9 and 0xA to one and zero, publish the work pointer at 0x74,
 *           and increment work byte 0xB. Advance work byte 1 after the calls.
 *           Otherwise reset work byte 9 to four and decrement work byte 1.
 * @status partial
 * @match 88.44
 * @residual 130 of 147 instructions match at the original 588-byte size.
 *           Action-slot index arithmetic and work-pointer registers differ.
 */
void decrementWorkByteARecordCountersBy2ThenQueueActionOnCountdownZeroBmagicMagic04303_801EF0B4(void) {
    u32 slot;
    u32 offset;
    u8 *record;
    u8 *arm_work;

    D_80145BD6[g_battle_work[0xA]].counter_00 -= 2;
    D_80145BD6[g_battle_work[0xA]].counter_01 -= 2;
    D_80145BD6[g_battle_work[0xA]].counter_02 -= 2;
    if (--g_battle_work[9] == 0) {
        if (g_battle_work[0xB] < 2) {
            func_80196670(g_battle_work[0xA]);
            func_800AF814(D_80146384[0]);
            if (D_801463C0 == 0x2B) {
                if ((u8)func_800AED5C(1, D_80146384[0]) != 0) {
                    slot = (u8)func_801E590C(1, 0x48);
                    offset = slot * 0x78u;
                    record = g_battle_work;
                    D_801EC334[offset] = 0;
                } else {
                    slot = (u8)func_801E590C(1, 0x48);
                    offset = slot * 0x78u;
                    record = g_battle_work;
                    D_801EC334[offset] = 8;
                }
                D_801EC339[offset] = 1;
                D_801EC33A[offset] = 0;
                arm_work = g_battle_work;
                *(u8 **)&D_801EC3A4[offset] = record;
                arm_work[0xB]++;
            }
            g_battle_work[1]++;
        } else {
            g_battle_work[9] = 4;
            g_battle_work[1]--;
        }
    }
}
