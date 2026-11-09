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

/* Template blob that the two activations of this overlay hand to the shared
 * activator; its role beyond being the activation template is unknown. */
extern u8 D_801FCF5C[]; /* @source 0x801FCF5C @kind unknown */

u8 func_8015CA48(void);
void func_801A4D54(void *arg0);
s32 func_8019651C(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* @source 0x801F754C
 * @behavior Entry 0 of the work-byte-1 handler table at 0x801FCF50; it selects
 * the pair of 0x98-byte work records this overlay activates. It asks the shared
 * slot allocator func_8015CA48 for a free identifier and stores it at cursor
 * byte 3 of the work object published at 0x1F800044, returning when that
 * identifier is 0xFF. Otherwise it marks the record the identifier scales as
 * live, asks func_8015CA48 for a second identifier at cursor byte 4, and when
 * that one is 0xFF releases the first record again and returns. With two
 * identifiers it marks the second record live, publishes each identifier as the
 * halfword at 0x1F800000 and hands both activations the local template at
 * 0x801FCF5C through func_801A4D54, republishing the work object at 0x1F800044,
 * spreads the two records' 0x34 and 0x38 accumulators by -0x1000/+0x1000, sets
 * their 0x20 flag bit, clears their 0x5C byte, submits both through
 * func_8019651C with the 0xF mask and mode 1, and finally sets cursor byte 9 to
 * 4 and cursor byte 1 to 1.
 * @status partial
 * @match 91.30
 * @residual This lane's gate measured 210 of 229 instructions and 920 against
 * 916 bytes. The first difference is the frame, which this build reserves as
 * 0x30 with 24 bytes below the save area where the original reserves 0x38 with
 * 32, shifting every save offset. The extra eight bytes are never addressed:
 * the original's only stack references are the five saves, the two 0x10
 * outgoing argument stores and the epilogue, so they are dead local space in
 * the original source. The body itself is proven by the span comparison: this
 * span and its span-identical sibling emi/world00/area032/13 at 0x801F30EC
 * differ in one internal jump word shifted by the span delta and in the
 * template address 0x801FCF5C alone, and that sibling's own recorded gate
 * holds the same 210 of 229 under the same compiler with the same frame first
 * difference. Measured and rejected this lane: a two-record u8 *records[2]
 * array carrying the submitted record pointers (189/236, 944 bytes), and an
 * unprototyped five-argument callee declaration (210/230, 920 bytes, frame
 * unchanged), which rules out the outgoing argument area as the source of the
 * eight bytes for those scalar declarations. Two further probes changed only
 * the first accumulator update to a 0x98-strided field view: a struct member
 * and a pointer-to-array view each measured 169/229 and 916 bytes, retaining
 * the 0x30 frame. Both were reverted. Baseline RTL under the owning compiler
 * contained no standalone orphan USE through global allocation. The tail also
 * schedules the func_8019651C argument constants and cursor loads differently;
 * the frame difference alone does not establish that scheduling cause. The
 * source construct behind the unused frame extent remains unknown. Further
 * compiler evidence and independent review are needed; the clean-C ladder is
 * not claimed exhausted.
 */
void claimTwoWorkRecordsActivateBothWithScriptSpreadAccumulatorsAndSubmitScenarioScena0300_801F754C(void) {
  u32 offset;
  s32 accum;
  u8 value;
  u8 *cursor;
  u8 *pair;
  u16 *slot;

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
  slot = &D_1F800000;

  *slot = cursor[3];
  func_801A4D54(&D_801FCF5C);
  *slot = cursor[4];
  func_801A4D54(&D_801FCF5C);
  D_1F800044 = cursor;

  offset = cursor[3] * 0x98;
  accum = *(s32 *)(D_801468BC + offset);
  accum -= 0x1000;
  *(s32 *)(D_801468BC + offset) = accum;
  offset = cursor[3] * 0x98;
  accum = *(s32 *)(D_801468C0 + offset);
  accum += 0x1000;
  *(s32 *)(D_801468C0 + offset) = accum;
  offset = cursor[4] * 0x98;
  accum = *(s32 *)(D_801468BC + offset);
  accum += 0x1000;
  *(s32 *)(D_801468BC + offset) = accum;
  offset = cursor[4] * 0x98;
  accum = *(s32 *)(D_801468C0 + offset);
  accum -= 0x1000;
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
