#include "bof3/bof3.h"

extern u8 D_80145FD0[]; /* @source 0x80145FD0 @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80144F5B; /* @source 0x80144F5B @kind unknown */
extern s16 D_80145FFE; /* @source 0x80145FFE @kind unknown */
extern s16 D_80146000; /* @source 0x80146000 @kind unknown */
extern u8 D_801F4060[]; /* @source 0x801F4060 @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F3A18
 * @behavior Publishes the second shared work base D_80145FD0 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 4 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the party-indexed byte of this payload's
 *           byte table at 0x801F4060 (indexed by the second shared party-id
 *           byte D_80144F5B) and halfwords +0x2E/+0x30 =
 *           D_80145FFE/D_80146000; unless the call reports 0xFF the returned
 *           record index is stored in work byte +0x0B. Takes no arguments and
 *           returns nothing. No jal reaches this address: its only in-image
 *           reference is the first word of the code-pointer run at 0x801F4D40
 *           in this payload's D_801F3FE4 textbin blob, so the state dispatcher
 *           that owns that run enters it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this body is instruction-identical to
 * the exact sibling
 * publishSecondWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld02Area07714_801F5E04
 * at 0x801F5E04 in emi/world02/area077/14 - the two shipped 0x80-byte ranges
 * differ in exactly two bytes, the low half of the `lbu` displacement, 0x4060
 * here against 0x62EC there, which is this payload's own mode table - so its
 * statement order, operand forms, declaration style and the reload of the
 * scratchpad cell after the call are taken from that sibling, and only this
 * function's table binding is written here. That reload matters: the returned
 * index lands in the record the callee saw rather than in the u8* copy
 * published before it. The byte table at 0x801F4060 sits past the last
 * code-pointer word of the D_801F3FE4 run and binds at its own address, the
 * same way the neighbouring 0x801F4D64 halfword table and 0x801F4DEC phase
 * table of this payload are bound.
 */
void publishSecondWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld01Area06813_801F3A18(
    void) {
  u8 result;

  D_1F800044 = D_80145FD0;
  result = func_8015CB18(4, 0, D_801F4060[D_80144F5B], D_80145FFE, D_80146000);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
