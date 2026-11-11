#include "bof3/bof3.h"

/* Third shared work base: the 0x140-byte-stride group the installer reaches
 * after the second base 0x80145FD0, published here as the scratchpad cursor.
 * @source 0x80146110 @kind unknown
 */
extern u8 D_80146110[];
/* Scratchpad work-pointer cell: the original re-materialises the cell after the
 * call instead of keeping the published pointer in a register, so the cell
 * keeps its plain global form and is evaluated at both uses.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Third shared party selector byte: the index the seeders of the third work
 * base read their mode table with.
 * @source 0x80144F5C @kind unknown
 */
extern u8 D_80144F5C;
/* The published base's own +0x2E halfword. @source 0x8014613E @kind unknown */
extern s16 D_8014613E;
/* The published base's own +0x30 halfword. @source 0x80146140 @kind unknown */
extern s16 D_80146140;
/* Mode table the seeders of this overlay share: the first of the three
 * twelve-byte rows at 0x801F4660/0x801F466C/0x801F4678 inside the 0x801F4624
 * tail blob of this payload.
 * @source 0x801F4660 @kind table
 */
extern u8 D_801F4660[];

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F3D70
 * @behavior Publishes the third shared work base D_80146110 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 3 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the byte D_801F4660[D_80144F5C] and
 *           halfwords +0x2E/+0x30 = D_8014613E/D_80146140, which are that
 *           base's own +0x2E/+0x30; unless the call reports 0xFF the returned
 *           record index is stored in work byte +0x0B of the record the cell
 *           points at after the call. Takes no arguments and returns nothing,
 *           and the 0x20-byte frame holds the outgoing fifth argument at +0x10
 *           and the return address at +0x18.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x80-byte body is instruction
 * identical to the exact sibling publishThirdWorkBaseAndSeedFrontEndRecord
 * Kind3FromModeTableWorld03Area13413_801F5350 at 0x801F5350 of the area134/13
 * payload except for one word, the `lbu` offset 0x60 here against 0x68 there,
 * which is this payload's own mode-table row; a `cmp` of the two 0x80-byte
 * spans shows no other difference, so statement order, operand forms, offset
 * spelling, declaration style and the re-read of the scratchpad cell after the
 * call are taken from that sibling and only this function's own table binding
 * is written here. The differing word is an immediate, so the clone lever has
 * no byte-identical source anywhere and is declined. Within this payload the
 * same stream appears at 0x801F3DF0 and 0x801F3E70: each differs from this
 * body in exactly two words, the kind literal (4 and 5 against 3) and the mode
 * table offset (0x6C and 0x78 against 0x60), so they are shape evidence and
 * not twins. No jal in this payload names 0x801F3D70, so the overlay reaches
 * this handler from outside its payload.
 */
void publishThirdWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld03Area11713_801F3D70(
    void) {
  u8 result;

  D_1F800044 = D_80146110;
  result = func_8015CB18(3, 0, D_801F4660[D_80144F5C], D_8014613E, D_80146140);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
