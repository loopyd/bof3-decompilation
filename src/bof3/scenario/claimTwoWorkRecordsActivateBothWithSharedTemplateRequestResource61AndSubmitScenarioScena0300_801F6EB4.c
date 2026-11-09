#include "bof3/bof3.h"

/* Scratchpad cursor cell every overlay step reads its work slot from, and the
 * scratchpad halfword that publishes the slot index being activated. */
extern u8 *D_1F800044;
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* 0x98-byte work-slot array of the shared main-executable data segment. The
 * allocator at 0x8015CA48 hands out its free indices and reports 0xFF when none
 * is left. The 0x34 and 0x38 words and the 0x5C byte of one slot are addressed
 * through their own canonical labels so the compiler emits the original
 * per-access `lui %hi` / `addu $at,$at,index` / `%lo($at)` triple instead of one
 * shared base. */
extern u8 workRecordBytesScenarioScena0300_80146888[]; /* @source 0x80146888 @kind data */
extern u8 D_801468BC[]; /* @source 0x801468BC @kind data */
extern u8 D_801468C0[]; /* @source 0x801468C0 @kind data */
extern u8 D_801468E4[]; /* @source 0x801468E4 @kind data */

/* 16-byte activation entry at 0x801FCF30, the second word of the per-frame
 * handler table region of this overlay; both activations of this body hand this
 * same entry to the shared activator. */
extern u8 D_801FCF30[]; /* @source 0x801FCF30 @kind data */

u8 func_8015CA48(void);
void func_801A65A0(u8 *entry);
void func_8014D6DC(s32 id, s32 mode);
s32 func_8019651C(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* @source 0x801F6EB4
 * @behavior Entry 0 of the per-frame handler table at 0x801FCF24; it selects
 * the pair of 0x98-byte work records this overlay animates. It asks the shared
 * slot allocator func_8015CA48 for a free identifier and stores it at cursor
 * byte 3 of the work object published at 0x1F800044, returning when that
 * identifier is 0xFF. Otherwise it marks the record the identifier scales as
 * live, asks func_8015CA48 for a second identifier at cursor byte 4, and when
 * that one is 0xFF releases the first record again and returns. With two
 * identifiers it marks the second record live, publishes each identifier as the
 * halfword at 0x1F800000 and activates it through func_801A65A0 with the local
 * 16-byte entry at 0x801FCF30, requesting resource 0x61 with mode 0xC after
 * each activation, republishes the work object at 0x1F800044, spreads the two
 * records apart by adding 0x1000 to the first 0x34 word and subtracting 0x1000
 * from its 0x38 word while giving the second record the mirrored -0x1000 and
 * +0x1000, sets their 0x20 flag bit, clears their 0x5C byte, submits both
 * through func_8019651C with the 0xF mask and mode 1, and finally sets cursor
 * byte 9 to 4 and cursor byte 1 to 1. It takes no arguments and returns
 * nothing.
 * @status partial
 * @match 96.61
 * @residual This lane's gate measured 228 of 235 instructions and 944 against
 * 940 bytes, and the frame, both record selections, the activation pair, the
 * mirrored accumulator spread, both flag stores, both 0x5C clears, both
 * submissions and the closing cursor stores agree. The first difference is the
 * `beq` of the first 0xFF test, whose absolute target sits one instruction
 * later only because this build's tail carries one `nop` the original fills;
 * the whole residual is that tail block, where the original hoists `li a1,15`
 * into the load-delay slot of the second record selector and this build leaves
 * that slot empty and places the same constant, `move a2,zero`, `move a3,zero`
 * and `li s2,1` one opportunity later, so those slots are transposed and the
 * second `D_801468E4` clear is reordered around the pair[3] load. The eight-byte
 * local above is the frame lever alone: without it the build's 0x30 frame makes
 * the prologue the first difference at 216 of 235, and sixteen bytes overshoot
 * to 0x40. Measured and rejected this lane at 177 of 236 each: the `|=` form,
 * the single-expression `value | 0x20` form, the re-read form and two separate
 * value locals, so the reused loaded-value local is required. The residual class
 * is scheduler placement inside one basic block, not a statement order this
 * source leaves open, and the nearest sibling
 * claimTwoWorkRecordsActivateBothWithScriptSpreadAccumulatorsAndSubmitScenarioScena0300_801F754C
 * recorded the same tail placement as its own residual.
 */
void claimTwoWorkRecordsActivateBothWithSharedTemplateRequestResource61AndSubmitScenarioScena0300_801F6EB4(void) {
  u32 offset;
  s32 accum;
  u8 value;
  u8 *cursor;
  u8 *entry;
  u16 *slot;
  u8 *pair;
  /* Unreferenced 8-byte local required by the original frame extent: this body
   * alone builds a 0x30 frame whose first difference is the prologue, while
   * one declared aggregate of these eight bytes restores the original 0x38. The
   * bytes constrain only that size; the name and type are a reconstruction
   * choice, so the declaration is flagged for review. */
  u8 frameSlack[8];

  D_1F800044[3] = func_8015CA48();
  if (D_1F800044[3] == 0xFF) {
    return;
  }

  offset = D_1F800044[3] * 0x98;
  workRecordBytesScenarioScena0300_80146888[offset] = 1;
  D_1F800044[4] = func_8015CA48();
  if (D_1F800044[4] == 0xFF) {
    offset = D_1F800044[3] * 0x98;
    workRecordBytesScenarioScena0300_80146888[offset] = 0;
    return;
  }

  offset = D_1F800044[4] * 0x98;
  workRecordBytesScenarioScena0300_80146888[offset] = 1;

  cursor = D_1F800044;
  entry = D_801FCF30;
  slot = &D_1F800000;

  *slot = cursor[3];
  func_801A65A0(entry);
  func_8014D6DC(0x61, 0xC);
  *slot = cursor[4];
  func_801A65A0(entry);
  func_8014D6DC(0x61, 0xC);
  D_1F800044 = cursor;

  offset = cursor[3] * 0x98;
  accum = *(s32 *)(D_801468BC + offset);
  accum += 0x1000;
  *(s32 *)(D_801468BC + offset) = accum;
  offset = cursor[3] * 0x98;
  accum = *(s32 *)(D_801468C0 + offset);
  accum -= 0x1000;
  *(s32 *)(D_801468C0 + offset) = accum;
  offset = cursor[4] * 0x98;
  accum = *(s32 *)(D_801468BC + offset);
  accum -= 0x1000;
  *(s32 *)(D_801468BC + offset) = accum;
  offset = cursor[4] * 0x98;
  accum = *(s32 *)(D_801468C0 + offset);
  accum += 0x1000;
  *(s32 *)(D_801468C0 + offset) = accum;

  offset = cursor[3] * 0x98;
  value = workRecordBytesScenarioScena0300_80146888[offset];
  value |= 0x20;
  workRecordBytesScenarioScena0300_80146888[offset] = value;
  offset = D_1F800044[4] * 0x98;
  value = workRecordBytesScenarioScena0300_80146888[offset];
  value |= 0x20;
  workRecordBytesScenarioScena0300_80146888[offset] = value;

  pair = D_1F800044;
  offset = pair[4] * 0x98;
  D_801468E4[offset] = 0;
  offset = pair[3] * 0x98;
  D_801468E4[offset] = 0;

  func_8019651C(workRecordBytesScenarioScena0300_80146888 + cursor[3] * 0x98, 0xF, 0, 0, 1);
  func_8019651C(workRecordBytesScenarioScena0300_80146888 + cursor[4] * 0x98, 0, 0, 0xF, 1);

  D_1F800044[9] = 4;
  D_1F800044[1] = 1;
}
