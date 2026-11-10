#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_8014638C; /* @source 0x8014638C @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern u8 D_801EC330[]; /* @source 0x801EC330 @kind data */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC364[]; /* @source 0x801EC364 @kind data */
extern u8 D_801EC368[]; /* @source 0x801EC368 @kind data */
extern u8 D_801EC36C[]; /* @source 0x801EC36C @kind data */
extern u8 D_801EC37C[]; /* @source 0x801EC37C @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */
extern void func_8015D71C(u16 arg0); /* @source 0x8015D71C */

/* @source 0x801EECFC
 * @behavior Claims two 0x78-byte queued slots through helper 0x801E590C(1, 13),
 *           truncating each returned index to a byte without checking failure.
 *           Saves the record pointer from 0x8014638C before either call. Clears
 *           first-slot bytes 1 and 9, publishes the scratchpad work pointer in
 *           its word 0x74 and copies record 0x801EB4E0 words 0x34/0x38/0x3C,
 *           adding 0x2000000 to the last. Links the second slot word 0x74 to
 *           the first slot, sets its byte 1 to 1, clears byte 9, publishes the
 *           reloaded work pointer in word 0x4C, and copies the saved record's
 *           word triplet with 0x1000000 added to the last. Calls 0x8015D71C
 *           with 0x100, clears work byte 0xB and increments work byte 1 through
 *           separate pointer-cell reads. Takes no arguments and returns nothing.
 *           Record fields and helper roles beyond these accesses remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueTwoLinkedSlotsCopyRecordTripletsThenInvokeHelperAt100AndAdvanceWorkByte1BmagicMagic07803_801EECFC(void) {
    u8 *savedRecord;
    u8 *record;
    u8 *firstWork;
    u8 *secondWork;
    u32 first;
    u32 second;

    savedRecord = D_8014638C;
    first = (u8)func_801E590C(1, 13) * 0x78;
    firstWork = g_battle_work;
    D_801EC331[first] = 0;
    D_801EC339[first] = 0;
    record = D_801EB4E0;
    *(u8 **)&D_801EC3A4[first] = firstWork;
    *(u32 *)&D_801EC364[first] = *(u32 *)(record + 0x34);
    *(u32 *)&D_801EC368[first] = *(u32 *)(record + 0x38);
    *(u32 *)&D_801EC36C[first] = *(u32 *)(record + 0x3C) + 0x2000000;
    second = (u8)func_801E590C(1, 13) * 0x78;
    first = (u32)&D_801EC330[first];
    secondWork = g_battle_work;
    *(u32 *)&D_801EC3A4[second] = first;
    D_801EC331[second] = 1;
    D_801EC339[second] = 0;
    *(u8 **)&D_801EC37C[second] = secondWork;
    *(u32 *)&D_801EC364[second] = *(u32 *)(savedRecord + 0x34);
    *(u32 *)&D_801EC368[second] = *(u32 *)(savedRecord + 0x38);
    *(u32 *)&D_801EC36C[second] = *(u32 *)(savedRecord + 0x3C) + 0x1000000;
    func_8015D71C(0x100);
    g_battle_work[0xB] = 0;
    g_battle_work[1]++;
}
