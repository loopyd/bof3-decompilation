#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern void func_8015D71C(u16 arg0); /* @source 0x8015D71C */

/* @source 0x801EFB74
 * @behavior Decrements scratchpad work-object byte 9 modulo 256. On zero,
 * calls the helper at 0x8015D71C with 0x100, copies shared record words
 * 0x34, 0x38 and 0x3C into the work object, and clears work byte 0xB.
 * Then sets work byte 9 to 0x14, clears byte 0xA and increments byte 2.
 * Reloads the scratchpad pointer after the triplet stores and each later
 * byte store. The local table word at 0x801F1278 names this handler.
 * Takes no arguments and returns nothing; field roles remain offset-based.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenInvokeHelperCopyRecordTripletClearByteBRearmByte9ClearByteAAndAdvanceWorkByte2BmagicMagic12603_801EFB74(void) {
    u8 *record;
    u8 *work;

    if (--D_1F800044[9] == 0) {
        func_8015D71C(0x100);
        record = D_801EB4E0;
        work = D_1F800044;
        *(u32 *)(work + 0x34) = *(u32 *)(record + 0x34);
        *(u32 *)(work + 0x38) = *(u32 *)(record + 0x38);
        *(u32 *)(work + 0x3C) = *(u32 *)(record + 0x3C);
        work[0xB] = 0;
        D_1F800044[9] = 0x14;
        D_1F800044[0xA] = 0;
        D_1F800044[2]++;
    }
}
