#include "bof3/bof3.h"

extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */
extern u8 D_80144F5A; /* @source 0x80144F5A @kind unknown */
extern s16 D_80145EBE; /* @source 0x80145EBE @kind unknown */
extern s16 D_80145EC0; /* @source 0x80145EC0 @kind unknown */
extern u8 D_801F51B8[]; /* @source 0x801F51B8 @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F4B88
 * @behavior Publishes the shared work base D_80145E90 as the scratchpad work
 *           cursor at 0x1F800044 and then seeds one front-end record of kind 1
 *           through the shared allocator/seeder func_8015CB18 with word +0x0C =
 *           0, word +0x10 = the mode-indexed byte of this payload's byte table
 *           at 0x801F51B8 and halfwords +0x2E/+0x30 =
 *           D_80145EBE/D_80145EC0; unless the call reports 0xFF the returned
 *           record index is stored in work byte +0x0B, the byte the byte-2
 *           handlers of this overlay read as their flag-number index. Takes no
 *           arguments and returns nothing. No jal reaches this address: its only
 *           in-image reference is the code-pointer word at 0x801F5990 in this
 *           payload's D_801F511C textbin blob, where it follows the three data
 *           pointers 0x801F595C/0x801F5968/0x801F5974 and directly precedes the
 *           six-byte bounds records starting at 0x801F5994.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x80-byte body is instruction
 * identical to the exact sibling func_801F32B8 at 0x801F32B8 in
 * emi/world00/area026/13 - the two ranges differ in exactly one word, the `lbu`
 * displacement 0x51B8 here against 0x4CE0 there, which is this payload's own
 * mode table - so its statement order, operand forms and declaration style are
 * taken from that sibling and the kind-1 family source
 * publishWorkBaseAndSeedFrontEndRecordKind1FromModeTableWorld02Area07714_801F5F84;
 * only this function's literals, passed constants and symbol bindings are
 * written here. The scratchpad cell is re-read after the call, so the index
 * lands in the record the callee saw rather than in the published copy.
 */
void publishWorkBaseAndSeedFrontEndRecordKind1FromModeTableWorld03Area11813_801F4B88(
    void) {
  u8 result;

  D_1F800044 = D_80145E90;
  result = func_8015CB18(1, 0, D_801F51B8[D_80144F5A], D_80145EBE, D_80145EC0);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
