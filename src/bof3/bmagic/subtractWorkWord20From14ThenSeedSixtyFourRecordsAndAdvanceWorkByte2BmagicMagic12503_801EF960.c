#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */
extern u8 D_801F0604[]; /* @source 0x801F0604 @kind data */
extern void func_800A9610(u8 arg0, s32 arg1); /* @source 0x800A9610 */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */
extern u8 reserveFirstFreeRecordBmagicMagic12503_801F0524(void); /* @source 0x801F0524 */
extern int rand(void);

/* @source 0x801EF960
 * @behavior Decrements work word 0x20 by two unless it is one, then subtracts
 * that word from signed work word 0x14. When the result is below 0x30, calls
 * 0x800A9610 with the battle byte and 0x10 and 0x8015D71C with 0x101, then
 * claims 64 stride-0x78 records. Each record gets byte 1 cleared, byte 0xB
 * set to the pass index, word 0x74 set to the pointer at 0x801EB4E0, and
 * byte 9 set to (index / 6) * 8 + (rand() & 7) + 1. Increments the pointed
 * object's byte 0xB after each claim and advances work byte 2 after the loop.
 * The allocator's 0xFF answer is not tested. This no-argument callback is
 * referenced by the pointer at payload offset 0x19F0, not by a direct call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void subtractWorkWord20From14ThenSeedSixtyFourRecordsAndAdvanceWorkByte2BmagicMagic12503_801EF960(void) {
    s32 *work;
    u8 *count;
    s32 *fields;
    u8 *owner;
    u8 index;
    u8 claimed;
    u32 offset;
    s32 value;

    work = (s32 *)g_battle_work;
    if (work[8] != 1) {
        work[8] -= 2;
    }
    fields = (s32 *)g_battle_work;
    fields[5] -= fields[8];
    if (fields[5] < 0x30) {
        func_800A9610(D_80146384, 0x10);
        func_8015D71C(0x101);
        for (index = 0; index < 0x40; index++) {
            claimed = reserveFirstFreeRecordBmagicMagic12503_801F0524();
            offset = claimed * 0x78;
            owner = D_801EB4E0;
            D_801F0604[offset + 1] = 0;
            D_801F0604[offset + 0xB] = index;
            *(u8 **)(D_801F0604 + offset + 0x74) = owner;
            value = ((index / 6u) * 8) + (rand() & 7) + 1;
            D_801F0604[offset + 9] = value;
            count = D_801EB4E0;
            count[0xB]++;
        }
        count = g_battle_work;
        count[2]++;
    }
}
