#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */
extern void func_80196718(void *record); /* @source 0x80196718 @kind function */
extern s32 func_8019651C(void *record, s32 arg1, s32 arg2, s32 arg3, s32 arg4); /* @source 0x8019651C @kind function */

/* @source 0x801EF444
 * @behavior Decrements scratchpad work byte 9 and selects the 0x140-byte
 * record at base 0x80145E90 indexed by work byte 4. When the stored countdown
 * is zero, submits the record through 0x80196718, calls 0x8019651C with
 * arguments record/0/0/0/1, stores its low return byte in work byte 0xA,
 * resets byte 9 to 8 and increments byte 2. Reloads the work pointer for
 * each store group after the calls. Entry 1 of the local byte-2 handler
 * table at 0x801EEC14; takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSubmitIndexedRecordStoreEffectByteAResetCountdownAndAdvanceWorkByte2BmagicMagic16903_801EF444(void) {
    u8 *work;
    u8 countdown;
    u8 *record;

    work = g_battle_work;
    countdown = --work[9];
    record = D_80145E90 + work[4] * 0x140u;
    if (countdown == 0) {
        func_80196718(record);
        g_battle_work[0xA] = func_8019651C(record, 0, 0, 0, 1);
        g_battle_work[9] = 8;
        g_battle_work[2]++;
    }
}
