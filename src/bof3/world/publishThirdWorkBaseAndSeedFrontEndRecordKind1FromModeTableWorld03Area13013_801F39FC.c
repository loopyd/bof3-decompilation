#include "bof3/bof3.h"

/* Third shared work base of this overlay: the 0x140-byte-stride group the
 * installer reaches after the second base 0x80145FD0, published here as the
 * scratchpad cursor.
 * @source 0x80146110 @kind unknown
 */
extern u8 D_80146110[];
/* Scratchpad work-pointer cell, bound by this target map. The original
 * re-materialises the cell after the seeder call, so the read after the call is
 * a fresh load and not a copy of the pointer published before it.
 * @source 0x1F800044 @kind data
 */
extern u8* workRecordCursor1F800044;
/* Third shared party selector byte: the index the seeders of the third work
 * base read their mode table with.
 * @source 0x80144F5C @kind unknown
 */
extern u8 D_80144F5C;
/* The published base's own +0x2E halfword. @source 0x8014613E @kind unknown */
extern s16 D_8014613E;
/* The published base's own +0x30 halfword. @source 0x80146140 @kind unknown */
extern s16 D_80146140;
/* Mode table the six seeders of this overlay share: byte 0x28 inside the
 * 0x801F3ED8 tail blob. Bound at that interior word through the generated row.
 * @source 0x801F3F00 @kind table
 */
extern u8 D_801F3F00[];

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F39FC
 * @behavior Publishes the third shared work base D_80146110 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 1 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the byte D_801F3F00[D_80144F5C] and
 *           halfwords +0x2E/+0x30 = D_8014613E/D_80146140, which are that
 *           base's own +0x2E/+0x30; unless the call reports 0xFF the returned
 *           record index is stored in work byte +0x0B. Takes no arguments and
 *           returns nothing. No jal reaches this address in this payload, so it
 *           is entered indirectly through the overlay's handler-pointer run.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x80-byte body is instruction
 * identical to the exact sibling publishThirdWorkBaseAndSeedFrontEndRecord
 * Kind1FromModeTableWorld03Area13413_801F546C at 0x801F546C in the area134
 * payload - same published base 0x80146110, same selector byte 0x80144F5C and
 * same forwarded +0x2E/+0x30 pair - except for the low half of the lbu
 * displacement of the per-party mode table, which encodes each payload's own
 * table base (0x801F3F00 here, 0x801F5868 there), so statement order, operand
 * forms, offset spelling and declaration style are taken from that sibling and
 * only this function's own table binding and cell spelling are written here.
 * The 0x80-byte pattern occurs exactly once across the shipped payloads, at
 * this address, so the clone lever has no byte-identical source and is
 * declined. The five other seeders of this payload at 0x801F36BC, 0x801F37F0,
 * 0x801F3870, 0x801F38F0 and 0x801F397C differ in the base, the selector byte
 * or the record kind, so they are shape evidence only. The published base,
 * selector and forwarded pair belong to the same channel: 0x80146110 is the
 * third of the three 0x140-byte-stride work bases (0x80145E90, 0x80145FD0,
 * 0x80146110), 0x80144F5C is the third of the three shared party bytes, and
 * base+0x2E/base+0x30 land exactly on 0x8014613E/0x80146140.
 */
void publishThirdWorkBaseAndSeedFrontEndRecordKind1FromModeTableWorld03Area13013_801F39FC(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80146110;
  result = func_8015CB18(1, 0, D_801F3F00[D_80144F5C], D_8014613E, D_80146140);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
