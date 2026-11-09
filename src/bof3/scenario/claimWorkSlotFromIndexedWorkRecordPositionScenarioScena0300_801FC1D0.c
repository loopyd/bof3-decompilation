#include "bof3/bof3.h"

/* 0x98-byte work-slot array of the shared main-executable data segment. The
 * allocator at 0x8015CA48 hands out its free indices and reports 0xFF when none
 * is left, and the fields of one slot are addressed through their own canonical
 * labels so the compiler emits the original per-access `lui %hi` /
 * `addu $at,$at,index` / `store %lo($at)` triple instead of one shared base. */
extern u8 workRecordBytesScenarioScena0300_80146888[]; /* @source 0x80146888 @kind data */
extern u8 D_801468FF[];
extern u8 D_80146900[];
extern u8 D_80146904[];
extern u8 D_80146908[];
extern u8 D_8014690C[];
extern u8 D_80146910[];
extern u8 D_80146914[];

/* Scratchpad cursor cell that every overlay step reads its current work slot
 * from, and the shared main-executable copy of the same pointer. */
extern u8 *D_1F800044;
extern u8 *D_80146884;

/* Shared 0x140-byte work-record array of the main executable, entered at the
 * two words holding one record's world coordinates. */
extern u8 D_80145EC4[];
extern u8 D_80145EC8[];

s32 func_8015CA48(void);
void func_801A4AA8(void);
void func_8014DD3C(s32 entry);
s32 func_8015477C(s32 x, s32 y);
void func_801A4990(const u8 *data);
void func_801A4914(void);

/* @source 0x801FC1D0
 * @behavior Spawns the overlay object into a free work slot, placed at the
 * position of the shared work record the argument selects. It asks the shared
 * slot allocator func_8015CA48 for a free slot index and returns 0 when that
 * index is 0xFF. Otherwise it resolves the 0x98-byte slot of 0x80146888 that the
 * index scales, publishes that slot pointer both at the scratchpad cursor
 * 0x1F800044 and at the shared copy 0x80146884, resets the slot through
 * func_801A4AA8, requests entry 0x67 through func_8014DD3C and sets slot flag
 * byte 0. The low byte of the argument then selects a 0x140-byte record of the
 * shared work-record array, and only the two leading words of that record are
 * consumed: the 0x34 word, biased by 0x8000, becomes slot word 0x34 and the 0x38
 * word becomes slot word 0x38, both mirrored into the slot words 0x80 and 0x84
 * while 0x88 is cleared. Slot halfword 0x3E receives the distance the shared
 * helper func_8015477C computes from those two coordinates, slot byte 1 is set
 * to state 4, bytes 2, 6 and 8 are cleared, slot halfword 0x8E is cleared, slot
 * bytes 0x77 and 0x78 are set to 2 and slot halfwords 0x7C and 0x8C are cleared.
 * It then hands a one-byte descriptor holding 0x88 to func_801A4990, raises flag
 * bit 0x20 of the slot flag byte, clears the three signed tint channels 0x5D,
 * 0x5E and 0x5F, sets the slot flags_5C byte to 1, calls func_801A4914 and
 * returns 1. The 0x30-byte frame keeps $ra and $s0-$s3, which hold the slot byte
 * offset, the slot pointer, the reused 1 constant and the argument.
 * @status partial
 * @match 94.21
 * @residual Live audit 114/121 instructions; 484 original bytes versus 484
 * current, so the size and every stored value agree and nine `D_1F800044`
 * pointer reads appear in both bodies. Two adjacent scheduling groups differ.
 * In the coordinate group the original issues the second shared work-record
 * word load after the 0x80146908 store, keeps that word in $v0 and emits a
 * second `lui $at,0x8014` for it, while this source issues the load one
 * statement earlier, keeps the word in $v1 and shares one `lui $at,0x8014`
 * between the load and the store. In the slot-byte group the original reuses
 * the $a0 pointer of the 0x01 store for the 0x3E store and puts the 0x02
 * pointer load between them so that load fills the store delay slot, while this
 * source loads the 0x02 pointer after the 0x3E store and keeps a `nop` there.
 * Residual class: scheduling and allocation. Representation, control flow,
 * widths, pointer cells and reload counts are proven and every operand
 * quantity agrees; the measured clean-C shapes are listed in the lane report.
 */
s32 claimWorkSlotFromIndexedWorkRecordPositionScenarioScena0300_801FC1D0(s32 arg0) {
  u8 *slot;
  u8 *work;
  s32 index;
  s32 offset;
  s32 position_offset;
  s32 coord_x;
  s32 coord_y;
  s32 distance;
  u8 descriptor;

  index = func_8015CA48();
  if ((u8)index == 0xFF) {
    return 0;
  }
  offset = (u8)index * 0x98;
  slot = &workRecordBytesScenarioScena0300_80146888[offset];
  D_1F800044 = slot;
  D_80146884 = slot;

  func_801A4AA8();
  func_8014DD3C(0x67);

  D_1F800044[0] = 1;

  position_offset = (u8)arg0 * 0x140;
  coord_x = PSX_REF(s32, D_80145EC4 + position_offset) + 0x8000;
  FIELD_REF(s32, D_1F800044, 0x34) = coord_x;
  PSX_REF(s32, D_80146908 + offset) = coord_x;
  coord_y = PSX_REF(s32, D_80145EC8 + position_offset);
  FIELD_REF(s32, D_1F800044, 0x38) = coord_y;
  PSX_REF(s32, D_8014690C + offset) = coord_y;
  PSX_REF(s32, D_80146910 + offset) = 0;

  distance = func_8015477C(FIELD_REF(s32, D_1F800044, 0x34),
                           FIELD_REF(s32, D_1F800044, 0x38));

  D_1F800044[1] = 4;
  FIELD_REF(u16, D_1F800044, 0x3E) = distance;
  D_1F800044[2] = 0;
  D_1F800044[8] = 0;
  D_1F800044[6] = 0;
  FIELD_REF(u16, slot, 0x8E) = 0;
  PSX_REF(u8, D_801468FF + offset) = 2;
  PSX_REF(u8, D_80146900 + offset) = 2;
  PSX_REF(u16, D_80146914 + offset) = 0;
  PSX_REF(u16, D_80146904 + offset) = 0;

  descriptor = 0x88;
  func_801A4990(&descriptor);

  D_1F800044[0] = D_1F800044[0] | 0x20;
  work = D_1F800044;
  work[0x5F] = 0;
  work[0x5E] = 0;
  work[0x5D] = 0;
  D_1F800044[0x5C] = 1;

  func_801A4914();
  return 1;
}
