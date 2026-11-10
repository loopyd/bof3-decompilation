#include "bof3/bof3.h"

extern void func_80196718(void *arg0);
extern s32 func_8019651C(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */
extern u8 D_801EB2E8[]; /* @source 0x801EB2E8 @kind unknown */

/* @source 0x801EF2C8
 * @behavior Selects a record using scratchpad work byte 4: indices below three
 *           use base 0x80145E90 and stride 0x140, others use base 0x801EB2E8
 *           and stride 0x118 without subtracting three. Decrements work byte 9
 *           and, when its stored byte becomes zero, submits the selected record
 *           through func_80196718 and calls func_8019651C with 0/0/0/1.
 *           Stores the low return byte in work byte 0xA, sets byte 9 to 8 and
 *           increments byte 2, reloading the work pointer for each store group.
 *           Entry 1 of the copied handler table used by the byte-2 dispatcher
 *           at 0x801EEFE0. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSubmitSelectedRecordStoreEffectByteASetByte9To8AndAdvanceWorkByte2BmagicMagic07403_801EF2C8(void) {
    u8 *record;
    u8 index;

    index = D_1F800044[4];
    if (index < 3u) {
        record = (u8 *)D_80145E90 + index * 0x140u;
    } else {
        record = D_801EB2E8 + index * 0x118u;
    }
    if (--D_1F800044[9] == 0) {
        func_80196718(record);
        D_1F800044[0xA] = func_8019651C(record, 0, 0, 0, 1);
        D_1F800044[9] = 8;
        D_1F800044[2]++;
    }
}
