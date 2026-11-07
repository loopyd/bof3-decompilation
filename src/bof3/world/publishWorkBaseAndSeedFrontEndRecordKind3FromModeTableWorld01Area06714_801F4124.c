#include "bof3/bof3.h"

extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80144F5A; /* @source 0x80144F5A @kind unknown */
extern s16 D_80145EBE; /* @source 0x80145EBE @kind unknown */
extern s16 D_80145EC0; /* @source 0x80145EC0 @kind unknown */
extern u8 D_801F4644[]; /* @source 0x801F4644 @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F4124
 * @behavior Publishes the shared work base D_80145E90 as the scratchpad work
 *           cursor at 0x1F800044 and then seeds one front-end record of kind 3
 *           through the shared allocator/seeder func_8015CB18 with word +0x0C =
 *           0, word +0x10 = the mode-indexed byte of this payload's byte table
 *           at 0x801F4644 and halfwords +0x2E/+0x30 = D_80145EBE/D_80145EC0;
 *           unless the call reports 0xFF the returned record index is stored in
 *           work byte +0x0B. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this body is instruction-identical to
 * the exact sibling publishWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld03Area13313_801F2CEC
 * at 0x801F2CEC in emi/world03/area133/13 - the two shipped 0x80-byte ranges
 * differ in exactly one word, the `lbu` displacement 0x4644 here against 0x2DD4
 * there, which is this payload's own mode table - so its statement order, operand
 * forms and declaration style are taken from that sibling (itself derived from
 * publishWorkBaseAndSeedFrontEndRecordFromModeTableWorld02Area07714_801F5AAC at
 * 0x801F5AAC) and only this function's symbol bindings are written here. The
 * table row is byte 0x20 of this payload's D_801F4624 blob, which opens with the
 * eight-entry code-pointer run of the area's state handlers, so 0x801F4644 is the
 * first of the three eight-byte mode tables. The scratchpad cell is re-read after
 * the call, so the index lands in the record the callee saw rather than in the
 * u8* copy published before it.
 */
void publishWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld01Area06714_801F4124(
    void) {
  u8 result;

  D_1F800044 = D_80145E90;
  result = func_8015CB18(3, 0, D_801F4644[D_80144F5A], D_80145EBE, D_80145EC0);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
