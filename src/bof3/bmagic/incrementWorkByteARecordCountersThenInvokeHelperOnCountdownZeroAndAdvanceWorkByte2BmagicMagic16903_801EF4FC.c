#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
typedef struct BmagicMagic16903CounterRecord {
    u8 counter_00;
    u8 counter_01;
    u8 counter_02;
    u8 unk_03[9];
} BmagicMagic16903CounterRecord;
extern BmagicMagic16903CounterRecord D_80145BD6[]; /* @source 0x80145BD6 @kind table */
extern void func_8015D71C(u16 selector); /* @source 0x8015D71C */

/* @source 0x801EF4FC
 * @behavior Increment the three leading bytes of the stride-12 record at
 *           0x80145BD6 selected by work byte 0xA, reloading the work pointer
 *           before each group. Decrement work byte 9 modulo 256. When it
 *           reaches zero, call helper 0x8015D71C with 0x101 if work byte 4 is
 *           odd, otherwise 0x102, then reload the work pointer and increment
 *           byte 2. Takes no arguments and returns nothing. The helper's role
 *           is unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteARecordCountersThenInvokeHelperOnCountdownZeroAndAdvanceWorkByte2BmagicMagic16903_801EF4FC(void) {
    s32 selector;

    D_80145BD6[g_battle_work[0xA]].counter_00++;
    D_80145BD6[g_battle_work[0xA]].counter_01++;
    D_80145BD6[g_battle_work[0xA]].counter_02++;
    if (--g_battle_work[9] == 0) {
        selector = 0x102;
        if (g_battle_work[4] & 1) {
            selector = 0x101;
        }
        func_8015D71C(selector);
        g_battle_work[2]++;
    }
}
