#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

typedef struct BmagicMagic07603CounterRecord {
    u8 counter_00;
    u8 counter_01;
    u8 counter_02;
    u8 unk_03[9];
} BmagicMagic07603CounterRecord;

extern BmagicMagic07603CounterRecord D_80145BD6[]; /* @source 0x80145BD6 @kind table */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */
extern void func_80196718(void *record); /* @source 0x80196718 @kind function */

/* @source 0x801EF160
 * @behavior Callback stored at payload word 0x801EEC14. Decrement the three
 *           leading bytes of the 12-byte record selected by work byte 0xA in
 *           table 0x80145BD6, re-reading the scratchpad work pointer for each
 *           counter. Decrement work byte 9 and test the truncated byte result.
 *           On zero, pass the pointer at 0x8014638C to helper 0x80196718,
 *           re-read the work pointer and increment work byte 1. No arguments
 *           or return value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteARecordCountersAndWorkByte9ThenSubmitRecordAndAdvanceWorkByte1OnZeroBmagicMagic07603_801EF160(void) {
    D_80145BD6[g_battle_work[0xA]].counter_00--;
    D_80145BD6[g_battle_work[0xA]].counter_01--;
    D_80145BD6[g_battle_work[0xA]].counter_02--;
    if (--g_battle_work[9] == 0) {
        func_80196718(D_8014638C);
        g_battle_work[1]++;
    }
}
