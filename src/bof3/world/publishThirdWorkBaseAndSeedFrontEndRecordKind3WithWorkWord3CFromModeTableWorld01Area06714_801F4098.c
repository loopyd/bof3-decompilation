#include "bof3/bof3.h"

extern u8 D_80146110[]; /* @source 0x80146110 @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80144F5C; /* @source 0x80144F5C @kind unknown */
extern s16 D_8014613E; /* @source 0x8014613E @kind unknown */
extern s16 D_80146140; /* @source 0x80146140 @kind unknown */
extern s32 D_8014614C; /* @source 0x8014614C @kind unknown */
extern u8 D_801F4644[]; /* @source 0x801F4644 @kind table */

/* Shared front-end record allocator and seeder at 0x8015CC00: the variant of
 * func_8015CB18 used by this payload's step-0/1/2 handlers. It takes the next
 * free 0x74-byte record index from func_8019601C and fills the same fields
 * (record byte +6 = arg0, word +0x0C = arg1, word +0x10 = arg2, halfword +0x2E
 * = arg3, halfword +0x30 = arg4) plus one extra word argument arg5, then
 * returns that index, or returns 0xFF without touching the table when it is
 * full. Only the call shape is known from this payload; arg5's field is the
 * callee's business.
 */
u8 func_8015CC00(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4, s32 arg5);

/* @source 0x801F4098
 * @behavior Publishes the third shared work base D_80146110 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 3 through the extended allocator/seeder func_8015CC00 with
 *           word +0x10 = the party-indexed byte of this payload's byte table at
 *           0x801F4644 (indexed by the third shared party-id byte D_80144F5C),
 *           halfwords +0x2E/+0x30 = D_8014613E/D_80146140 and the extra word
 *           argument = D_8014614C, the third base's word at +0x3C; unless the
 *           call reports 0xFF the returned record index is stored in work byte
 *           +0x0B. Takes no arguments and returns nothing. No jal reaches this
 *           address: its only in-image reference is the code-pointer word at
 *           0x801F5274, the third word of the handler run at 0x801F5260 inside
 *           this payload's D_801F4624 textbin blob, so the state dispatcher
 *           that owns that run enters it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x8C-byte body is instruction
 * identical to the exact sibling
 * publishSecondWorkBaseAndSeedFrontEndRecordKind3WithWorkWord3CFromModeTableWorld01Area06714_801F400C
 * at 0x801F400C in this same payload apart from which work base it publishes
 * and which cells it forwards, and its literal set is the one the five-argument
 * exact sibling publishThirdWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld01Area06714_801F4224
 * at 0x801F4224 already binds: the third 0x140-byte-stride work base
 * D_80146110, the third shared party-id byte D_80144F5C and that base's own
 * +0x2E/+0x30 halfword pair D_8014613E/D_80146140. The extended seeder
 * func_8015CC00 and the sixth stack-passed word argument come from the
 * 0x801F400C sibling, with the forwarded word being this base's +0x3C cell
 * D_8014614C, which is what keeps both stack arguments at 0x10/0x14($sp).
 * Statement order, operand forms, offset spelling and declaration style are
 * taken from the 0x801F400C sibling and only this function's own bindings are
 * written here. Nothing was cloned: one payload cmp over all out/binaries shows
 * no byte-identical twin of this span, so the two siblings are shape evidence.
 * The reload of the scratchpad cell after the call matters: the returned index
 * lands in the record the callee saw rather than in the u8* copy published
 * before it.
 */
void publishThirdWorkBaseAndSeedFrontEndRecordKind3WithWorkWord3CFromModeTableWorld01Area06714_801F4098(
    void) {
  u8 result;

  D_1F800044 = D_80146110;
  result = func_8015CC00(3, 0, D_801F4644[D_80144F5C], D_8014613E, D_80146140,
                         D_8014614C);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
