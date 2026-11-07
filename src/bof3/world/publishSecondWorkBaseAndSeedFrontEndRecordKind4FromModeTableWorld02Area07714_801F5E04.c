#include "bof3/bof3.h"

extern u8 D_80145FD0[]; /* @source 0x80145FD0 @kind unknown */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 D_80144F5B; /* @source 0x80144F5B @kind unknown */
extern s16 D_80145FFE; /* @source 0x80145FFE @kind unknown */
extern s16 D_80146000; /* @source 0x80146000 @kind unknown */
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

/* @source 0x801F5E04
 * @behavior Publishes the second shared work base D_80145FD0 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 4 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = D_801F62EC[D_80144F5B] and halfwords
 *           +0x2E/+0x30 = D_80145FFE/D_80146000; unless the call reports 0xFF
 *           the returned record index is stored in work byte +0x0B. Takes no
 *           arguments and returns nothing. No jal reaches this address: its only
 *           in-image reference is the code-pointer word at 0x801F6AAC in the
 *           state-pointer run of this payload's D_801F63C8 textbin blob, which
 *           lists this overlay's handlers in address order, so the state
 *           dispatcher reaches it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this body is instruction-identical to
 * the exact sibling publishSecondWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld02Area07714_801F5C80
 * at 0x801F5C80 except for two words of shipped literals - the record kind
 * (4 rather than 3) and the mode-table base (0x801F62EC rather than
 * 0x801F6B40) - so statement order, operand forms and declaration style are
 * taken from that sibling and only this function's own literals and bindings
 * are written from its bytes. The published work base, the party-id byte it
 * indexes the mode table with, the two forwarded halfwords and the scratchpad
 * publish/re-read spelling are already identical to that sibling, so they are
 * unchanged here. D_80145FD0 is the second of the three 0x140-byte-stride work
 * bases D_80145E90/D_80145FD0/D_80146110 that the boss overlay installer
 * 0x800C1B0C walks as its three channel groups, and D_80144F5B is the second of
 * the three shared party-id bytes at D_80144F5A, so base, selected mode-table
 * byte and the two forwarded +0x2E/+0x30 halfwords all belong to the same party
 * member. The table D_801F62EC is the byte-entry mode table that this payload's
 * neighbours 0x801F5E84 and 0x801F5F04 also index with the third and first
 * party-id bytes; it sits at byte offset 0x36EC of the payload, past the
 * dispatch-pointer rows of the reviewed D_801F6250 textbin region and 0xDC
 * bytes before the D_801F63C8 run, so it is an interior table of that run and
 * binds at its own word. Its party-indexed bytes 0x14/0x14/0x30/0x14 repeat the
 * shape of the D_801F6Bxx mode rows that the 0x801F5AAC..0x801F5D00 siblings
 * read, which is what identifies its role. The scratchpad cell is re-read after
 * the call, so the index lands in the record the callee saw rather than in the
 * u8* copy published before it.
 */
void publishSecondWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld02Area07714_801F5E04(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80145FD0;
  result = func_8015CB18(4, 0, D_801F62EC[D_80144F5B], D_80145FFE, D_80146000);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
