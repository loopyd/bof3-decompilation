#include "bof3/bof3.h"

extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 D_80144F5A; /* @source 0x80144F5A @kind unknown */
extern s16 D_80145EBE; /* @source 0x80145EBE @kind unknown */
extern s16 D_80145EC0; /* @source 0x80145EC0 @kind unknown */
extern u8 D_801F6B38[]; /* @source 0x801F6B38 @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full. The scenario
 * overlay seeder at 0x801FCD14 writes the same record fields directly, which
 * cross-checks this role.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F5C00
 * @behavior Publishes the shared work base D_80145E90 as the scratchpad work
 *           cursor at 0x1F800044 and then seeds one front-end record of kind 2
 *           through the shared allocator/seeder func_8015CB18 with word +0x0C =
 *           0, word +0x10 = D_801F6B38[D_80144F5A] and halfwords +0x2E/+0x30 =
 *           D_80145EBE/D_80145EC0; unless the call reports 0xFF the returned
 *           record index is stored in work byte +0x0B. Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this body differs from the exact
 * siblings of this target at 0x801F5AAC (kind 3, table 0x801F6B28) and
 * 0x801F5B2C (kind 4, table 0x801F6B30) in exactly two words of the shipped
 * 0x80-byte range, the record-kind immediate `addiu $a0,$zero,0x2` and the
 * `lbu` displacement `0x6B38` of its own mode-table row, so statement order,
 * operand forms and declaration style are taken from those siblings and only
 * the kind literal and table row are written from this function's bytes. The
 * three rows are adjacent 8-byte byte tables in the D_801F63C8 textbin blob of
 * this payload. The scratchpad cell is re-read after the call, so the index
 * lands in the record the callee saw rather than in the u8* copy published
 * before it.
 */
void publishWorkBaseAndSeedFrontEndRecordKind2FromModeTableWorld02Area07714_801F5C00(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80145E90;
  result = func_8015CB18(2, 0, D_801F6B38[D_80144F5A], D_80145EBE, D_80145EC0);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
