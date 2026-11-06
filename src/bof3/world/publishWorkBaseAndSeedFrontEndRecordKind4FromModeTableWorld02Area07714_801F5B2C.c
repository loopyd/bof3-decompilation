#include "bof3/bof3.h"

extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 D_80144F5A; /* @source 0x80144F5A @kind unknown */
extern s16 D_80145EBE; /* @source 0x80145EBE @kind unknown */
extern s16 D_80145EC0; /* @source 0x80145EC0 @kind unknown */
extern u8 D_801F6B30[]; /* @source 0x801F6B30 @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full. The scenario
 * overlay seeder at 0x801FCD14 writes the same record fields directly, which
 * cross-checks this role.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F5B2C
 * @behavior Publishes the shared work base D_80145E90 as the scratchpad work
 *           cursor at 0x1F800044 and then seeds one front-end record through
 *           the shared allocator/seeder func_8015CB18 with record kind 4,
 *           word +0x0C = 0, word +0x10 = D_801F6B30[D_80144F5A] and halfwords
 *           +0x2E/+0x30 = D_80145EBE/D_80145EC0; unless the call reports 0xFF
 *           the returned record index is stored in work byte +0x0B. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this body is instruction-identical to
 * the exact sibling of this target at 0x801F5AAC except for the record-kind
 * literal (4 rather than 3) and the mode-table row base (0x801F6B30 rather
 * than 0x801F6B28), the two words that differ in the shipped bytes. Both
 * handler rows sit next to each other in the overlay's state-pointer run at
 * 0x801F6A88/0x801F6A84. The scratchpad cell is re-read after the call, so the
 * index lands in the record the callee saw rather than in the u8* copy
 * published before it.
 */
void publishWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld02Area07714_801F5B2C(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80145E90;
  result = func_8015CB18(4, 0, D_801F6B30[D_80144F5A], D_80145EBE, D_80145EC0);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
