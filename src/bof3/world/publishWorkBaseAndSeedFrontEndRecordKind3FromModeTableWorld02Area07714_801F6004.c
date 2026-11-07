#include "bof3/bof3.h"

extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 D_80144F5A; /* @source 0x80144F5A @kind unknown */
extern s16 D_80145EBE; /* @source 0x80145EBE @kind unknown */
extern s16 D_80145EC0; /* @source 0x80145EC0 @kind unknown */
extern u8 D_801F6B58[]; /* @source 0x801F6B58 @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F6004
 * @behavior Publishes the shared work base D_80145E90 as the scratchpad work
 *           cursor at 0x1F800044 and then seeds one front-end record of kind 3
 *           through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = D_801F6B58[D_80144F5A] and halfwords
 *           +0x2E/+0x30 = D_80145EBE/D_80145EC0; unless the call reports 0xFF
 *           the returned record index is stored in work byte +0x0B. Takes no
 *           arguments and returns nothing. No jal reaches this address: its
 *           only in-image reference is the code-pointer word at 0x801F6ABC in
 *           the state-pointer run of this payload's D_801F63C8 textbin blob,
 *           which lists this overlay's handlers in address order, so the state
 *           dispatcher reaches it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this body is instruction-identical to
 * the exact sibling publishWorkBaseAndSeedFrontEndRecordKind1FromModeTableWorld02Area07714_801F5F84
 * at 0x801F5F84 except for two words - the record-kind literal (3 rather than
 * 1) and the %lo displacement of the mode-row load at 0x801F6040 (0x801F6B58
 * here, 0x801F6B50 there; the %hi word matches because both rows sit in the
 * 0x801F page) - so statement order, operand forms and declaration style are
 * taken from that sibling and only this function's own literals and table
 * binding are written from its bytes. The published work base, the party-id
 * byte it indexes the row with, the two forwarded +0x2E/+0x30 halfwords and the
 * scratchpad publish/re-read spelling are already identical to that sibling.
 * The row keeps the family's FromModeTable wording because 0x801F6B58 has
 * exactly one reader in this payload, unlike the shared 0x801F62EC mode row.
 * Its seven party-indexed bytes equal the 0x801F6B50 row of the same base, yet
 * the two rows are read with different record kinds, so the row is not a copy
 * of that one. It sits at byte offset 0x3F58 of the payload, 0x20 bytes before
 * the end of the D_801F63C8 textbin run, so it is an interior table of that run
 * and binds at its own word. The scratchpad cell is re-read after the call, so
 * the index lands in the record the callee saw rather than in the u8* copy
 * published before it.
 */
void publishWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld02Area07714_801F6004(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80145E90;
  result = func_8015CB18(3, 0, D_801F6B58[D_80144F5A], D_80145EBE, D_80145EC0);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
