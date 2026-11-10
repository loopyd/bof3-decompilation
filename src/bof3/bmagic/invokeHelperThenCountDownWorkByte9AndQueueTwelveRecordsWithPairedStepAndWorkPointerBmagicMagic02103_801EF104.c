#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 magic02103RecordFlags[]; /* @source 0x801EFE14 @kind data */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC @kind function */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C @kind function */
extern u8 reserveFirstFreeRecordBmagicMagic02103_801EFD4C(void);

/* @source 0x801EF104
 * @behavior Calls 0x8014DAEC, ignores its result, and decrements work byte 9.
 *           When that byte reaches zero, calls 0x8015D71C with 0x100, clears
 *           work bytes 9 and 0xB, and claims twelve records from the local
 *           0x78-stride pool without checking for allocation failure. Each
 *           record receives 1 + 3 * (ordinal / 2) in byte 9 and the current
 *           work pointer in word 0x74. Increments work byte 0xB after each
 *           claim, then advances work byte 2. Takes no arguments and returns
 *           nothing. Payload word 0x801EEC14 names this handler. Record and
 *           helper roles beyond these accesses remain offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperThenCountDownWorkByte9AndQueueTwelveRecordsWithPairedStepAndWorkPointerBmagicMagic02103_801EF104(void) {
    u8 ordinal;
    u8 index;
    u32 offset;
    u8 *work;
    u8 *counter;

    func_8014DAEC();
    if (--g_battle_work[9] == 0) {
        func_8015D71C(0x100);
        g_battle_work[9] = 0;
        g_battle_work[0xB] = 0;
        for (ordinal = 0; ordinal < 12; ordinal++) {
            index = reserveFirstFreeRecordBmagicMagic02103_801EFD4C();
            offset = index * 0x78;
            work = g_battle_work;
            magic02103RecordFlags[offset + 9] = (ordinal >> 1) * 3 + 1;
            counter = g_battle_work;
            *(u32 *)&magic02103RecordFlags[offset + 0x74] = (u32)work;
            counter[0xB]++;
        }
        g_battle_work[2]++;
    }
}
