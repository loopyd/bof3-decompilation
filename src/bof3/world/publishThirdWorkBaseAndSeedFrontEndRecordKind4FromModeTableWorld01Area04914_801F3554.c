#include "bof3/bof3.h"

extern u8 D_80146110[]; /* @source 0x80146110 @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80144F5C; /* @source 0x80144F5C @kind unknown */
extern s16 D_8014613E; /* @source 0x8014613E @kind unknown */
extern s16 D_80146140; /* @source 0x80146140 @kind unknown */
extern u8 D_801F398C[]; /* @source 0x801F398C @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F3554
 * @behavior Publishes the third shared work base D_80146110 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 4 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = D_801F398C[D_80144F5C] and halfwords
 *           +0x2E/+0x30 = D_8014613E/D_80146140; unless the call reports 0xFF
 *           the returned record index is stored in work byte +0x0B. Takes no
 *           arguments and returns nothing. No jal reaches this address: its only
 *           in-image reference is the code-pointer word at 0x801F5C64, index 12
 *           of the 32-entry state-pointer run at 0x801F5C34 in this payload's
 *           D_801F398C textbin blob, which lists this overlay's handlers in
 *           address order, so the state dispatcher reaches it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the shipped 0x80-byte range is
 * instruction-identical to the exact sibling
 * publishThirdWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld02Area07714_801F5E84
 * at 0x801F5E84 in emi/world02/area077/14 - 126 of its 128 bytes are equal and
 * the two that differ are the low half of the `lbu` displacement, 0x398C here
 * against 0x62EC there, which is this payload's own mode table - so its
 * statement order, operand forms, declaration style and the reload of the
 * scratchpad cell after the call are taken from that sibling, and only this
 * function's table binding is written from these bytes. The published work base
 * D_80146110, the party-id byte D_80144F5C and the two forwarded halfwords
 * D_8014613E/D_80146140 are already identical to that sibling, which is the
 * third-base member of the same kind-4 pair: D_80146110 is the third of the
 * three 0x140-byte-stride work bases D_80145E90/D_80145FD0/D_80146110 that the
 * boss overlay installer walks as its three channel groups and D_80144F5C is
 * the third of the three shared party-id bytes at D_80144F5A, so the base, the
 * selected table byte and the +0x2E/+0x30 halfwords all belong to one party
 * member. The table is this payload's byte-entry mode table at the head of the
 * D_801F398C textbin region, which the neighbouring functions 0x801F3454,
 * 0x801F34D4 and 0x801F376C index the same way, so it binds at the region
 * start. The scratchpad cell is re-read after the call, so the index lands in
 * the record the callee saw rather than in the u8* copy published before it.
 */
void publishThirdWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld01Area04914_801F3554(
    void) {
  u8 result;

  D_1F800044 = D_80146110;
  result = func_8015CB18(4, 0, D_801F398C[D_80144F5C], D_8014613E, D_80146140);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
