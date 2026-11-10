#include "bof3/bof3.h"

/* Second shared work base of this overlay: the 0x140-byte-stride group the
 * installer walks after 0x80145E90, published here as the scratchpad cursor.
 * @source 0x80145FD0 @kind unknown
 */
extern u8 D_80145FD0[];
/* Scratchpad work-pointer cell, bound by this target map. The original
 * re-materialises the cell after the seeder call, so the read after the call is
 * a fresh load and not a copy of the pointer published before it.
 * @source 0x1F800044 @kind data
 */
extern u8* workRecordCursor1F800044;
/* Second shared party selector byte: the index the seeders of the second work
 * base read their mode table with.
 * @source 0x80144F5B @kind unknown
 */
extern u8 D_80144F5B;
/* The published base's own +0x2E halfword. @source 0x80145FFE @kind unknown */
extern s16 D_80145FFE;
/* The published base's own +0x30 halfword. @source 0x80146000 @kind unknown */
extern s16 D_80146000;
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

/* @source 0x801F397C
 * @behavior Publishes the second shared work base D_80145FD0 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 1 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the byte D_801F3F00[D_80144F5B] and
 *           halfwords +0x2E/+0x30 = D_80145FFE/D_80146000, which are that
 *           base's own +0x2E/+0x30; unless the call reports 0xFF the returned
 *           record index is stored in work byte +0x0B. Takes no arguments and
 *           returns nothing. No jal reaches this address in this payload, so it
 *           is entered indirectly through the overlay's handler-pointer run.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x80-byte body is instruction
 * identical to the exact sibling
 * publishSecondWorkBaseAndSeedFrontEndRecordKind1FromModeTableWorld03Area13413
 * _801F53EC at 0x801F53EC in the area134 payload except for one word - the low
 * half of the lbu displacement of the per-party mode table, which encodes each
 * payload's own table base (0x801F3F00 here, 0x801F5868 there) - so statement
 * order, operand forms, offset spelling and declaration style are taken from
 * that sibling and only this function's own table binding is written here. The
 * exact 0x80-byte pattern appears once in the 396 shipped payloads, at this
 * address, and the nearest differing spans store their own literals, so the
 * clone lever has no byte-identical source and is declined; the discriminator
 * makes the match a shape match rather than a copy. The five other seeders of
 * this payload at 0x801F36BC, 0x801F37F0, 0x801F3870, 0x801F39FC and further
 * on differ in the base, the selector byte or the record kind, so they are
 * shape evidence only. The published base, selector and forwarded pair belong
 * to the same channel: D_80145FD0 is the second of the three 0x140-byte-stride
 * work bases, 0x80144F5B is the second of the three shared party bytes, and the
 * record kind is the fact that separates this member from 0x801F37F0.
 */
void publishSecondWorkBaseAndSeedFrontEndRecordKind1FromModeTableWorld03Area13013_801F397C(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80145FD0;
  result = func_8015CB18(1, 0, D_801F3F00[D_80144F5B], D_80145FFE, D_80146000);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
