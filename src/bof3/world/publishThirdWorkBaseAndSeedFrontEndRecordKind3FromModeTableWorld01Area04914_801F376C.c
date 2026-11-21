#include "bof3/bof3.h"

extern u8 D_80146110[]; /* @source 0x80146110 @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80144F5C; /* @source 0x80144F5C @kind unknown */
extern s16 D_8014613E; /* @source 0x8014613E @kind unknown */
extern s16 D_80146140; /* @source 0x80146140 @kind unknown */
extern u8 D_801F398C[]; /* @source 0x801F398C @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F376C
 * @behavior Publishes the third shared work base D_80146110 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 3 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = D_801F398C[D_80144F5C] and halfwords
 *           +0x2E/+0x30 = D_8014613E/D_80146140; unless the call reports 0xFF
 *           the returned record index is stored in work byte +0x0B of the
 *           record the cell points at after the call. Takes no arguments and
 *           returns nothing, and the 0x20-byte frame holds the outgoing fifth
 *           argument at +0x10 and the return address at +0x18.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x80-byte body differs from the
 * exact same-target sibling
 * publishThirdWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld01Area04914_801F3554
 * at 0x801F3554 in one word only, the kind literal at +0x2C, 0x03000424 here
 * against 0x04000424 there; a `cmp` of the two 0x80-byte spans shows every
 * other byte equal, so statement order, operand forms, offset spelling,
 * declaration style and the re-read of the scratchpad cell after the call are
 * taken from that sibling and only this function's own kind literal is written
 * here. The differing word is an immediate, and a scan of every payload under
 * out/binaries finds no byte-identical copy of this span, so the clone lever is
 * declined. The kind-3 member of the same family in another payload,
 * publishThirdWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld03Area11713_801F3D70
 * at 0x801F3D70, differs from this span in one word as well, the low half of the
 * `lbu` displacement, 0x398C here against 0x4660 there, which is each payload's
 * own mode table; it confirms the kind-3 statement set and is not byte
 * identical either. The mode table is this payload's byte-entry blob at the
 * 0x801F398C textbin region start, which the neighbouring functions 0x801F3454,
 * 0x801F34D4 and 0x801F3554 index through D_80144F5C the same way. No jal in
 * this payload names 0x801F376C: its single in-image reference is the
 * code-pointer word at 0x801F5C80, entry 19 of the 32-entry state-pointer run
 * at 0x801F5C34 in the same blob, so the state dispatcher reaches this handler
 * indirectly from outside the payload.
 */
void publishThirdWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld01Area04914_801F376C(
    void) {
  u8 result;

  D_1F800044 = D_80146110;
  result = func_8015CB18(3, 0, D_801F398C[D_80144F5C], D_8014613E, D_80146140);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
