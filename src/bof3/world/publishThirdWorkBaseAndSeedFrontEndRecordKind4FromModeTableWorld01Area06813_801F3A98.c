#include "bof3/bof3.h"

extern u8 D_80146110[]; /* @source 0x80146110 @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80144F5C; /* @source 0x80144F5C @kind unknown */
extern s16 D_8014613E; /* @source 0x8014613E @kind unknown */
extern s16 D_80146140; /* @source 0x80146140 @kind unknown */
extern u8 D_801F4060[]; /* @source 0x801F4060 @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F3A98
 * @behavior Publishes the third shared work base D_80146110 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 4 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the party-indexed byte of this payload's
 *           byte table at 0x801F4060 (indexed by the third shared party-id byte
 *           D_80144F5C) and halfwords +0x2E/+0x30 =
 *           D_8014613E/D_80146140; unless the call reports 0xFF the returned
 *           record index is stored in work byte +0x0B. Takes no arguments and
 *           returns nothing. No jal reaches this address: its only in-image
 *           reference is the second code-pointer word of the handler run at
 *           0x801F4D40, at 0x801F4D44 in this payload's D_801F3FE4 textbin
 *           blob, so the state dispatcher that owns that run enters it
 *           indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x80-byte body is instruction-
 * identical to the exact siblings
 * publishThirdWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld02Area07714_801F5E84
 * at 0x801F5E84 in emi/world02/area077/14 and, one ordinal and one party byte
 * earlier, publishSecondWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld01Area06813_801F3A18
 * at 0x801F3A18 in this same payload. Against the world02 sibling the two
 * shipped ranges differ in exactly two bytes, the low half of the `lbu`
 * displacement, 0x4060 here against 0x62EC there, which is this payload's own
 * table, so its statement order and declaration style are taken from that
 * sibling and only this function's table and shared-base bindings are written
 * here. The reload of the scratchpad cell after the call matters: the returned
 * index lands in the record the callee saw rather than in the u8* copy
 * published before it. The byte table at 0x801F4060 is bound at its own address
 * the same way this payload's neighbour 0x801F3A18 already binds it.
 */
void publishThirdWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld01Area06813_801F3A98(
    void) {
  u8 result;

  D_1F800044 = D_80146110;
  result = func_8015CB18(4, 0, D_801F4060[D_80144F5C], D_8014613E, D_80146140);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
