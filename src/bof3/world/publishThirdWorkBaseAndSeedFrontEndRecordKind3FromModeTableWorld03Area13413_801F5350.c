#include "bof3/bof3.h"

/* Third shared work base of this overlay: the 0x140-byte-stride group the
 * installer reaches after the second base 0x80145FD0, published here as the
 * scratchpad cursor.
 * @source 0x80146110 @kind unknown
 */
extern u8 D_80146110[];
/* Scratchpad work-pointer cell: the original re-materialises the cell for the
 * publish and again after the call, so the cell keeps its volatile form.
 * @source 0x1F800044 @kind data
 */
extern u8* volatile D_1F800044;
/* Third shared party selector byte: the index the seeders of the third work
 * base read their mode table with.
 * @source 0x80144F5C @kind unknown
 */
extern u8 D_80144F5C;
/* The published base's own +0x2E halfword. @source 0x8014613E @kind unknown */
extern s16 D_8014613E;
/* The published base's own +0x30 halfword. @source 0x80146140 @kind unknown */
extern s16 D_80146140;
/* Mode table the six seeders of this overlay share: byte 0x144 inside the
 * 0x801F5724 tail blob. Bound by the target map and the generated row.
 * @source 0x801F5868 @kind table
 */
extern u8 D_801F5868[];

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F5350
 * @behavior Publishes the third shared work base D_80146110 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 3 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the byte D_801F5868[D_80144F5C] and
 *           halfwords +0x2E/+0x30 = D_8014613E/D_80146140, which are that
 *           base's own +0x2E/+0x30; unless the call reports 0xFF the returned
 *           record index is stored in work byte +0x0B. Takes no arguments and
 *           returns nothing. No jal reaches this address: the overlay only
 *           holds it in its tail data blob, so it is entered indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x80-byte body is instruction
 * identical to the exact sibling publishSecondWorkBaseAndSeedFrontEndRecord
 * Kind3FromModeTableWorld03Area13413_801F52D0 at 0x801F52D0 in this payload,
 * except for the published base, its own +0x2E/+0x30 pair, the selector byte
 * and the mode table - so statement order, operand forms, offset spelling,
 * declaration style and the re-read of the scratchpad cell after the call are
 * taken from that sibling and only this function's own bindings are written
 * here. The two bodies differ in their address immediates, so the clone lever
 * has no byte-identical source and is declined. Within this payload the same
 * stream appears at 0x801F53EC (second base, selector byte 0x80144F5B, kind 1)
 * and 0x801F546C (this base and selector, kind 1): both remain asm rows and
 * differ from this body in the kind literal, the forwarded halfwords or the
 * base, so they are shape evidence and not twins. The published base, selector
 * and forwarded pair belong to the same channel: D_80146110 is the third of
 * the 0x140-byte-stride work bases and 0x80144F5C the third of the three
 * shared party bytes.
 */
void publishThirdWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld03Area13413_801F5350(
    void) {
  u8 result;

  D_1F800044 = D_80146110;
  result = func_8015CB18(3, 0, D_801F5868[D_80144F5C], D_8014613E, D_80146140);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
