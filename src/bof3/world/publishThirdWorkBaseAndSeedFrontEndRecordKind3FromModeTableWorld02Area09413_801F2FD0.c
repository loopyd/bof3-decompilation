#include "bof3/bof3.h"

extern u8 D_80146110[]; /* @source 0x80146110 @kind unknown */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 D_80144F5C; /* @source 0x80144F5C @kind unknown */
extern s16 D_8014613E; /* @source 0x8014613E @kind unknown */
extern s16 D_80146140; /* @source 0x80146140 @kind unknown */
extern u8 func_801F3504[]; /* @source 0x801F3504 @kind data */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F2FD0
 * @behavior Publishes the third shared work base D_80146110 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 3 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the mode-indexed byte of the table at
 *           0x801F3504 (indexed by the shared party-id byte D_80144F5C) and
 *           halfwords +0x2E/+0x30 = D_8014613E/D_80146140; unless the call
 *           reports 0xFF the returned record index is stored in work byte
 *           +0x0B. Takes no arguments and returns nothing. No jal reaches this
 *           address: its only in-image reference is the code-pointer word at
 *           0x801F52B8, the third entry of the handler run 0x801F52B0 that
 *           lists 0x801F2ED0/0x801F2F50/0x801F2FD0/0x801F3050 in address order
 *           inside this payload's T_801F5204 textbin blob, so the state
 *           dispatcher reaches it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the 0x80-byte range is instruction
 * identical to the exact sibling
 * publishThirdWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld02Area07714_801F5D00
 * of target emi/world02/area077/14, a payload of this same 0x801F2C00 load
 * address, except for the low halfword of the one word that loads the mode-table
 * byte (0x801F3504 here, 0x801F6B48 there; a payload cmp over the two 0x80-byte
 * spans differs in exactly those two bytes), so statement order, operand forms
 * and declaration style are taken from that sibling and only this function's own
 * bindings are written from its bytes. The within-payload siblings
 * publishWorkBaseAndSeedFrontEndRecordKind2FromModeTableWorld02Area09413_801F2ED0
 * and publishSecondWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld02Area09413_801F2F50
 * show the same body over the first and second of the three work bases, party-id
 * bytes and forwarded halfword pairs, and the boss overlay installer 0x800C1B0C
 * walks D_80145E90/D_80145FD0/D_80146110 as its three channel groups, so
 * D_80146110, D_80144F5C and the two +0x2E/+0x30 halfwords D_8014613E/D_80146140
 * all belong to the same party member. The mode table is this payload's own
 * 0x801F3504 span, which the generated name spells as a function although the
 * original only ever loads from it as a table, so it binds as a u8 array at its
 * own address. The scratchpad cell is re-read after the call, so the index lands
 * in the record the callee saw rather than in the u8* copy published before it.
 */
void publishThirdWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld02Area09413_801F2FD0(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80146110;
  result = func_8015CB18(3, 0, func_801F3504[D_80144F5C], D_8014613E, D_80146140);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
