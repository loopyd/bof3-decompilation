#include "bof3/bof3.h"

extern u8 D_80145FD0[]; /* @source 0x80145FD0 @kind unknown */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 D_80144F5B; /* @source 0x80144F5B @kind unknown */
extern s16 D_80145FFE; /* @source 0x80145FFE @kind unknown */
extern s16 D_80146000; /* @source 0x80146000 @kind unknown */
extern u8 func_801F3504[]; /* @source 0x801F3504 @kind data */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F2F50
 * @behavior Publishes the second shared work base D_80145FD0 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 3 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the mode-indexed byte of the table at
 *           0x801F3504 (indexed by the shared party-id byte D_80144F5B) and
 *           halfwords +0x2E/+0x30 = D_80145FFE/D_80146000; unless the call
 *           reports 0xFF the returned record index is stored in work byte
 *           +0x0B. Takes no arguments and returns nothing. No jal reaches this
 *           address: its only in-image reference is the code-pointer word at
 *           0x801F52B4, the second entry of the handler run 0x801F52B0 that
 *           lists 0x801F2ED0/0x801F2F50/0x801F2FD0/0x801F3050 in address order
 *           inside this payload's T_801F5204 textbin blob, so the state
 *           dispatcher reaches it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the 0x80-byte range is instruction
 * identical to the exact sibling
 * publishWorkBaseAndSeedFrontEndRecordKind2FromModeTableWorld02Area09413_801F2ED0
 * at 0x801F2ED0 of this same payload except for five words of shipped literals
 * (payload cmp of the two spans: the published base low halfword, the party-id
 * lbu displacement, both forwarded lh displacements and the record-kind
 * immediate), so statement order, operand forms and declaration style are taken
 * from that sibling and only this function's own literals are written from its
 * bytes. The cross-target sibling
 * publishSecondWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld02Area07714_801F5E04
 * carries the same work base and party-id byte with kind 4 and its own payload's
 * mode table, which places D_80145FD0 second of the three work bases
 * D_80145E90/D_80145FD0/D_80146110 and D_80144F5B second of the three party-id
 * bytes at D_80144F5A. The mode table is this payload's own 0x801F3504 span,
 * which the generated name spells as a function although the original only ever
 * loads from it as a table, so it binds as a u8 array at its own address. The
 * scratchpad cell is re-read after the call, so the index lands in the record
 * the callee saw rather than in the u8* copy published before it.
 */
void publishSecondWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld02Area09413_801F2F50(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80145FD0;
  result = func_8015CB18(3, 0, func_801F3504[D_80144F5B], D_80145FFE, D_80146000);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
