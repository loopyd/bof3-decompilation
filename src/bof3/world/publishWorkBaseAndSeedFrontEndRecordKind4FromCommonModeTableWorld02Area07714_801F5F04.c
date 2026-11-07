#include "bof3/bof3.h"

extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 D_80144F5A; /* @source 0x80144F5A @kind unknown */
extern s16 D_80145EBE; /* @source 0x80145EBE @kind unknown */
extern s16 D_80145EC0; /* @source 0x80145EC0 @kind unknown */
extern u8 D_801F62EC[]; /* @source 0x801F62EC @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full. The scenario
 * overlay seeder at 0x801FCD14 writes the same record fields directly, which
 * cross-checks this role.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F5F04
 * @behavior Publishes the shared work base D_80145E90 as the scratchpad work
 *           cursor at 0x1F800044 and then seeds one front-end record of kind 4
 *           through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = D_801F62EC[D_80144F5A] and halfwords
 *           +0x2E/+0x30 = D_80145EBE/D_80145EC0; unless the call reports 0xFF
 *           the returned record index is stored in work byte +0x0B. Takes no
 *           arguments and returns nothing. No jal reaches this address: its
 *           only in-image reference is the code-pointer word at 0x801F6AB4 in
 *           the state-pointer run of this payload's D_801F63C8 textbin blob,
 *           which lists this overlay's handlers in address order, so the state
 *           dispatcher reaches it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this body is instruction-identical to
 * the exact sibling publishWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld02Area07714_801F5B2C
 * at 0x801F5B2C except for one word, the %lo displacement of the mode-row load
 * at 0x801F5F40 (0x801F62EC here, 0x801F6B30 there; the %hi word matches
 * because both rows sit in the 0x801F page), so statement order, operand forms
 * and declaration style are taken from that sibling and only this function's
 * own table binding is written from its bytes. The published work base, the
 * party-id byte it indexes the row with, the two forwarded +0x2E/+0x30
 * halfwords and the scratchpad publish/re-read spelling are already identical
 * to that sibling, so they are unchanged here. The name keeps the family
 * pattern publish<ordinal>WorkBaseAndSeedFrontEndRecordKind<N>From<row>Table
 * and marks the row as the common one because 0x801F62EC is the mode row all
 * three kind-4 seeds of this payload read - the first, second and third work
 * bases at 0x801F5F04, 0x801F5E04 and 0x801F5E84 - while each 0x801F6Bxx row
 * this family also reads has exactly one reader. Its party-indexed bytes
 * 0x14/0x14/0x30/0x14 differ from the 0x801F6B30 row of the same published
 * base only at the third entry (0x30 against 0x28). It sits at byte offset
 * 0x36EC of the payload, past the dispatch-pointer rows of the reviewed
 * D_801F6250 textbin region and 0xDC bytes before the D_801F63C8 run, so it is
 * an interior table of that run and binds at its own word. The scratchpad cell
 * is re-read after the call, so the index lands in the record the callee saw
 * rather than in the u8* copy published before it.
 */
void publishWorkBaseAndSeedFrontEndRecordKind4FromCommonModeTableWorld02Area07714_801F5F04(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80145E90;
  result = func_8015CB18(4, 0, D_801F62EC[D_80144F5A], D_80145EBE, D_80145EC0);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
