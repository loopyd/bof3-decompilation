#include "bof3/bof3.h"

extern u8 D_80145FD0[]; /* @source 0x80145FD0 @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80144F5B; /* @source 0x80144F5B @kind unknown */
extern s16 D_80145FFE; /* @source 0x80145FFE @kind unknown */
extern s16 D_80146000; /* @source 0x80146000 @kind unknown */
extern s32 D_8014600C; /* @source 0x8014600C @kind unknown */
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

/* @source 0x801F400C
 * @behavior Publishes the second shared work base D_80145FD0 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 3 through the extended allocator/seeder func_8015CC00 with
 *           word +0x10 = the mode-indexed byte of this payload's byte table at
 *           0x801F4644, halfwords +0x2E/+0x30 = D_80145FFE/D_80146000 and the
 *           extra word argument = D_8014600C, the same base's word at +0x3C;
 *           unless the call reports 0xFF the returned record index is stored in
 *           work byte +0x0B. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x8C-byte body is the five-argument
 * exact sibling publishSecondWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld01Area06714_801F41A4
 * at 0x801F41A4 plus one extra forwarded word, so its statement order, operand
 * forms, offset spelling and declaration style come from that sibling and only
 * this function's own bindings are written here. The sibling seeds through
 * func_8015CB18 with the pair D_80145FFE/D_80146000; here the call is
 * func_8015CC00 (the sibling's callee +0xE8, the extended seeder of the same
 * module) with the same pair plus the `lw` of D_8014600C as a sixth, stack-passed
 * argument, which is what grows the body from 0x80 to 0x8C bytes and moves both
 * stack arguments to 0x10/0x14($sp). D_80145FD0, D_80144F5B and
 * D_80145FFE/D_80146000 are unchanged from the sibling; D_8014600C is that
 * base's own +0x3C word, so the published base, the party-id index byte, the
 * forwarded pair and the extra word all belong to the same channel. The payload's
 * handler run at 0x801F526C reaches this body as entry 1, between the first-base
 * extended seed 0x801F3F80 and the third-base one 0x801F4098, which confirms the
 * three extended seeds cover the three work bases in order. Nothing was cloned:
 * the ranges differ in their literals and argument count, so the sibling is
 * shape evidence only.
 */
void publishSecondWorkBaseAndSeedFrontEndRecordKind3WithWorkWord3CFromModeTableWorld01Area06714_801F400C(
    void) {
  u8 result;

  D_1F800044 = D_80145FD0;
  result = func_8015CC00(3, 0, D_801F4644[D_80144F5B], D_80145FFE, D_80146000,
                         D_8014600C);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
