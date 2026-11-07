#include "bof3/bof3.h"

extern u8 D_80146110[]; /* @source 0x80146110 @kind unknown */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 D_80144F5C; /* @source 0x80144F5C @kind unknown */
extern s16 D_8014613E; /* @source 0x8014613E @kind unknown */
extern s16 D_80146140; /* @source 0x80146140 @kind unknown */
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

/* @source 0x801F5E84
 * @behavior Publishes the third shared work base D_80146110 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 4 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = D_801F62EC[D_80144F5C] and halfwords
 *           +0x2E/+0x30 = D_8014613E/D_80146140; unless the call reports 0xFF
 *           the returned record index is stored in work byte +0x0B. Takes no
 *           arguments and returns nothing. No jal reaches this address: its only
 *           in-image reference is the code-pointer word at 0x801F6AB0 in the
 *           state-pointer run of this payload's D_801F63C8 textbin blob, which
 *           lists this overlay's handlers in address order, so the state
 *           dispatcher reaches it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this body is instruction-identical to
 * the exact sibling publishThirdWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld02Area07714_801F5D00
 * at 0x801F5D00 except for two words of shipped literals - the record kind
 * (4 rather than 3) and the mode-table row it indexes (0x801F62EC rather than
 * 0x801F6B48) - so statement order, operand forms and declaration style are
 * taken from that sibling and only this function's own literals and bindings
 * are written from its bytes. The published third work base D_80146110, the
 * third shared party-id byte D_80144F5C and the two forwarded halfwords
 * D_8014613E/D_80146140 are already identical to that sibling, which is also
 * the relation the exact siblings 0x801F5E04 and 0x801F5C80 show: 0x801F5E04
 * reaches the same 0x801F62EC row through the second party-id byte D_80144F5B
 * and the second work base D_80145FD0, so this function is the third-base
 * member of that kind-4 pair. D_80146110 is the third of the three
 * 0x140-byte-stride work bases D_80145E90/D_80145FD0/D_80146110 that the boss
 * overlay installer 0x800C1B0C walks as its three channel groups, and
 * D_80144F5C is the third of the three shared party-id bytes at D_80144F5A, so
 * base, selected mode-table byte and the two forwarded +0x2E/+0x30 halfwords
 * all belong to the same party member. The table D_801F62EC is the byte-entry
 * mode table that this payload's neighbours 0x801F5E04 and 0x801F5F04 also
 * index with the second and first party-id bytes; it sits at byte offset 0x36EC
 * of the payload, past the dispatch-pointer rows of the reviewed D_801F6250
 * textbin region and 0xDC bytes before the D_801F63C8 run, so it is an interior
 * table of that run and binds at its own word. The scratchpad cell is re-read
 * after the call, so the index lands in the record the callee saw rather than
 * in the u8* copy published before it.
 */
void publishThirdWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld02Area07714_801F5E84(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80146110;
  result = func_8015CB18(4, 0, D_801F62EC[D_80144F5C], D_8014613E, D_80146140);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
