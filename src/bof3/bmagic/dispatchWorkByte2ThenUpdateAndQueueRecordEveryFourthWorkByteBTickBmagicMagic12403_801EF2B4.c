#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F08E4[])(void); /* @source 0x801F08E4 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern u16 D_1F800008; /* @source 0x1F800008 @kind data */
extern u16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern u8 D_801F0901[]; /* @source 0x801F0901 @kind data */
extern u8 D_801F092E[]; /* @source 0x801F092E @kind data */
extern u8 D_801F0930[]; /* @source 0x801F0930 @kind data */
extern u8 D_801F0974[]; /* @source 0x801F0974 @kind data */
extern void func_801EF6E8(void); /* @source 0x801EF6E8 */
extern u8 reserveFirstFreeRecordBmagicMagic12403_801F0828(void); /* @source 0x801F0828 */

/* @source 0x801EF2B4
 * @behavior Dispatches work byte 2 through the table at 0x801F08E4. Reloads
 *           the scratchpad work pointer and, while work bytes 0 and 2 are
 *           nonzero, increments byte 0xB and calls the local helper at
 *           0x801EF6E8. Reloads the work pointer again and, when byte 0xB is
 *           divisible by four, byte 2 is 1 and byte 0xA is nonzero, reserves
 *           a record. Unless the allocator returns 0xFF, sets record byte 1
 *           to 1, publishes the pointer from 0x801EB4E0 at record offset
 *           0x74, copies scratchpad halfwords 8 and 0xA to record offsets
 *           0x2E and 0x30, then increments byte 0xB through a second read
 *           of 0x801EB4E0. The record pool begins at 0x801F0900 and has
 *           stride 0x78. Takes no arguments and returns nothing; work byte
 *           roles beyond these offsets and the helper's role are unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2ThenUpdateAndQueueRecordEveryFourthWorkByteBTickBmagicMagic12403_801EF2B4(void) {
    u8 *work;
    u8 *updatedWork;
    u8 *record;
    u8 *recordCount;
    u32 slot;
    u32 offset;

    D_801F08E4[g_battle_work[2]]();
    work = g_battle_work;
    if (work[0] != 0 && work[2] != 0) {
        work[0xB]++;
        func_801EF6E8();
        updatedWork = g_battle_work;
        if ((updatedWork[0xB] & 3) == 0 && updatedWork[2] == 1 && updatedWork[0xA] != 0) {
            slot = reserveFirstFreeRecordBmagicMagic12403_801F0828();
            if (slot != 0xFF) {
                offset = slot * 0x78;
                record = D_801EB4E0;
                D_801F0901[offset] = 1;
                *(u32 *)&D_801F0974[offset] = (u32)record;
                recordCount = D_801EB4E0;
                *(u16 *)&D_801F092E[offset] = D_1F800008;
                *(u16 *)&D_801F0930[offset] = D_1F80000A;
                recordCount[0xB]++;
            }
        }
    }
}
