#include "bof3/bof3.h"

extern u8 D_80145FD0[]; /* @source 0x80145FD0 @kind unknown */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 D_80144F5B; /* @source 0x80144F5B @kind unknown */
extern s16 D_80145FFE; /* @source 0x80145FFE @kind unknown */
extern s16 D_80146000; /* @source 0x80146000 @kind unknown */
extern u8 D_801F6B40[]; /* @source 0x801F6B40 @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full. The scenario
 * overlay seeder at 0x801FCD14 writes the same record fields directly, which
 * cross-checks this role.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F5C80
 * @behavior Publishes the second shared work base D_80145FD0 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 3 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = D_801F6B40[D_80144F5B] and halfwords
 *           +0x2E/+0x30 = D_80145FFE/D_80146000; unless the call reports 0xFF
 *           the returned record index is stored in work byte +0x0B. Takes no
 *           arguments and returns nothing. No jal reaches this address: its
 *           only in-image reference is the code-pointer word at 0x801F6A98 in
 *           the state-pointer run of this payload's D_801F63C8 textbin blob,
 *           which lists this overlay's handlers in address order, so the state
 *           dispatcher reaches it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this body is instruction-identical to
 * the exact sibling publishWorkBaseAndSeedFrontEndRecordFromModeTableWorld02Area07714_801F5AAC
 * at 0x801F5AAC except for five words of shipped literals - the published work
 * base, the shared party-id byte it indexes the mode table with, the two
 * halfwords it forwards to the seeder and that table's own row - so statement
 * order, operand forms and declaration style are taken from that sibling and
 * only this function's own literals and bindings are written from its bytes.
 * D_80145FD0 is the second of the three 0x140-byte-stride work bases
 * D_80145E90/D_80145FD0/D_80146110 that the boss overlay installer 0x800C1B0C
 * walks as its three channel groups, D_80144F5B is the second of the three
 * shared party-id bytes at D_80144F5A, and the two forwarded halfwords are that
 * base's own +0x2E/+0x30, so base and selected mode-table row belong to the
 * same party member. The table row sits in the D_801F63C8 textbin blob of this
 * payload. The scratchpad cell is re-read after the call, so the index lands in
 * the record the callee saw rather than in the u8* copy published before it.
 * The same-family stream of the world01/area038/13 payload at its own
 * 0x801F2C94 is not byte-identical to this one - it carries an extra selector
 * guard, its own allocator call and the inlined record writes - so it is shape
 * evidence only and nothing here was cloned from it.
 */
void publishSecondWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld02Area07714_801F5C80(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80145FD0;
  result = func_8015CB18(3, 0, D_801F6B40[D_80144F5B], D_80145FFE, D_80146000);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
