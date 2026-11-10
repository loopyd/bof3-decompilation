#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

typedef struct BmagicMagic08203CounterRecord {
    u8 counter_00;
    u8 counter_01;
    u8 counter_02;
    u8 unk_03[9];
} BmagicMagic08203CounterRecord;

extern BmagicMagic08203CounterRecord D_80145BD6[]; /* @source 0x80145BD6 @kind table */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */
extern u8 D_800B6BF0[]; /* @source 0x800B6BF0 @kind table */
extern u8 D_801EC334[]; /* @source 0x801EC334 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC33A[]; /* @source 0x801EC33A @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

extern void func_80196718(void *record); /* @source 0x80196718 */
extern void func_800AF814(u8 value); /* @source 0x800AF814 */
extern u32 func_800AED5C(u32 first, u32 second); /* @source 0x800AED5C */
extern u32 func_801E590C(u32 kind, u32 mode); /* @source 0x801E590C */

/* @source 0x801EF10C
 * @behavior Decrement the three bytes of the 12-byte counter record selected by
 *           work byte 0xA, then decrement work byte 9. On zero, submit the
 *           published record, invoke helper 0x800AF814 with byte 0x80146384,
 *           and test helper 0x800AED5C using the byte selected by work byte 4
 *           in table 0x800B6BF0 and byte 0x80146384. Claim an action slot with
 *           arguments 1 and 0x48. On a nonzero test, publish the work pointer,
 *           set slot byte 9 to 1, byte 0xA to 0 and byte 4 to work byte 4.
 *           Otherwise set byte 4 to 8, byte 9 to 1, byte 0xA to 0, then publish
 *           the work pointer. Finally increment work bytes 0xB and 1.
 * @status partial
 * @match 80.14
 * @residual 117 of 146 instructions match; the candidate is four bytes short.
 *           The action-slot arms reorder the pointer store and kind-byte read,
 *           use different registers for the scaled slot and work pointer, and
 *           leave the tail work-pointer load at the join rather than in each arm.
 */
void decrementWorkByteARecordCountersThenQueueActionOnCountdownZeroBmagicMagic08203_801EF10C(void) {
    u32 slot;
    u32 offset;
    u8 *record;

    D_80145BD6[g_battle_work[0xA]].counter_00--;
    D_80145BD6[g_battle_work[0xA]].counter_01--;
    D_80145BD6[g_battle_work[0xA]].counter_02--;
    if (--g_battle_work[9] == 0) {
        func_80196718(D_8014638C);
        func_800AF814(D_80146384);
        if ((u8)func_800AED5C(D_800B6BF0[g_battle_work[4]], D_80146384) != 0) {
            slot = (u8)func_801E590C(1, 0x48);
            offset = slot * 0x78u;
            record = g_battle_work;
            *(u8 **)&D_801EC3A4[offset] = record;
            D_801EC339[offset] = 1;
            D_801EC33A[offset] = 0;
            D_801EC334[offset] = record[4];
        } else {
            slot = (u8)func_801E590C(1, 0x48);
            offset = slot * 0x78u;
            record = g_battle_work;
            D_801EC334[offset] = 8;
            D_801EC339[offset] = 1;
            D_801EC33A[offset] = 0;
            *(u8 **)&D_801EC3A4[offset] = record;
        }
        g_battle_work[0xB]++;
        g_battle_work[1]++;
    }
}
