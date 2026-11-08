#include "bof3/bof3.h"

extern u8 D_80145FD0[]; /* @source 0x80145FD0 @kind unknown */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 D_80144F5B; /* @source 0x80144F5B @kind unknown */
extern s16 D_80145FFE; /* @source 0x80145FFE @kind unknown */
extern s16 D_80146000; /* @source 0x80146000 @kind unknown */
extern u8 D_801F3430[]; /* @source 0x801F3430 @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F3048
 * @behavior Publishes the second shared work base D_80145FD0 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 3 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the per-party byte table at 0x801F3430
 *           indexed by the second shared party byte D_80144F5B and halfwords
 *           +0x2E/+0x30 = D_80145FFE/D_80146000; unless the call reports 0xFF
 *           the returned record index is stored in work byte +0x0B. Takes no
 *           arguments and returns nothing. No jal reaches this address: its
 *           only in-image reference is the code-pointer word at 0x801F4374 in
 *           the handler-pointer run that starts at 0x801F4348 in this payload's
 *           tail data blob, which lists the overlay entry points in address
 *           order, so the state dispatcher reaches it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x80-byte body is instruction
 * identical to the exact sibling
 * publishSecondWorkBaseAndSeedFrontEndRecordKind1FromModeTableWorld03Area13613_801F2F50
 * at 0x801F2F50 in this same payload with one shipped word differing, the kind
 * immediate this one passes (3 here, 1 there); the remaining member of the
 * family in this payload, at 0x801F2E50, differs in that word as well as in the
 * high half of its per-party byte table displacement, so the kind-1 member is
 * this one's nearest exact sibling and supplies statement order, operand forms,
 * declaration style and the reload of the scratchpad cell after the call, while
 * only this function's own literal is written here. That reload matters: the
 * returned index lands in the record the callee saw rather than in the cursor
 * copy published before it. The two forwarded halfwords are the published
 * base's own +0x2E/+0x30, so base and selected table row belong to the same
 * party member. This family encodes its own table base and record kind in its
 * instruction words, so no member is byte-identical to another and the clone
 * lever does not apply. D_801F3430 is a byte table inside the tail data blob
 * that starts at 0x801F3424, twelve bytes past the kind-2 member's table head,
 * and it binds at its own address through the generated symbol row.
 */
void publishSecondWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld03Area13613_801F3048(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80145FD0;
  result = func_8015CB18(3, 0, D_801F3430[D_80144F5B], D_80145FFE, D_80146000);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
