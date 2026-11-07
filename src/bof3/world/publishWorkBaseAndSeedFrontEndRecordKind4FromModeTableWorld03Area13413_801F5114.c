#include "bof3/bof3.h"

/* First shared work base this overlay publishes as the scratchpad cursor.
 * @source 0x80145E90 @kind unknown
 */
extern u8 D_80145E90[];
/* Scratchpad work-pointer cell: the original re-materialises the cell for the
 * publish and again after the call, so the cell keeps its volatile form.
 * @source 0x1F800044 @kind data
 */
extern u8* volatile D_1F800044;
/* Shared party selector byte the four base-1/base-2/base-3 seeders of this
 * overlay read as their mode-table index (0xF5A/0xF5B/0xF5C).
 * @source 0x80144F5A @kind unknown
 */
extern u8 D_80144F5A;
/* @source 0x80145EBE @kind unknown */
extern s16 D_80145EBE;
/* @source 0x80145EC0 @kind unknown */
extern s16 D_80145EC0;
/* Byte table inside this payload's 0x801F5724 tail data blob, indexed by the
 * shared selector byte. @source 0x801F5874 @kind table
 */
extern u8 D_801F5874[];

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F5114
 * @behavior Publishes the first shared work base D_80145E90 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 4 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = D_801F5874[D_80144F5A] and halfwords
 *           +0x2E/+0x30 = D_80145EBE/D_80145EC0, which are the published
 *           base's own +0x2E/+0x30; unless the call reports 0xFF the returned
 *           record index is stored in work byte +0x0B. Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Name and shape evidence: this overlay ships six members of the same seeder
 * family, all reached only through the entry-pointer run at 0x801F6330 in the
 * tail data blob. They differ in exactly the literals this body carries: the
 * published base with its selector byte and forwarded halfword pair (D_80145E90
 * /0xF5A/0x5EBE/0x5EC0 here, 0x80145FD0/0xF5B/0x5FFE/0x6000 at 0x801F52D0 and
 * 0x801F53EC, 0x80146110/0xF5C/0x613E/0x6140 at 0x801F5350 and 0x801F546C) and
 * the record kind literal (4 here, 1 at 0x801F5200 and 0x801F53EC and
 * 0x801F546C, 3 at 0x801F52D0 and 0x801F5350). Only 0x801F5114 uses the
 * D_801F5874 mode-table row; the other five read D_801F5868, so base kind and
 * table row identify this member. The body is instruction-identical to the
 * exact sibling publishWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld02Area07714_801F5B2C
 * at 0x801F5B2C except for the mode-table displacement word (0x5874 here,
 * 0x6B30 there), so statement order, declaration style and the reload of the
 * scratchpad cell after the call are taken from that sibling and only this
 * function's own literals and bindings are written here. The reload matters:
 * the returned index lands in the record the callee saw rather than in the
 * cursor copy published before it. This family encodes its own table row and
 * record kind in its instruction words, so no member is byte-identical to
 * another and the cross-target clone lever does not apply; the shape is the
 * evidence.
 */
void publishWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld03Area13413_801F5114(
    void) {
  u8 result;

  D_1F800044 = D_80145E90;
  result = func_8015CB18(4, 0, D_801F5874[D_80144F5A], D_80145EBE, D_80145EC0);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
