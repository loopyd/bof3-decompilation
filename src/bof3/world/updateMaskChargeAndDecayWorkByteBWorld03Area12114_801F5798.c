#include "bof3/bof3.h"

/* @source 0x80145AA4 @kind unknown */
extern u16 D_80145AA4;
/* @source 0x80145AB6 @kind unknown */
extern u16 D_80145AB6;
/* @source 0x80143E6C @kind unknown */
extern u32 D_80143E6C;
/* @source 0x1F800044 @kind unknown */
extern u8* area12114WorkCursor_1F800044;

/* @source 0x801F5798
 * @behavior Decrements work byte 0x0A when nonzero. When it reaches zero,
 * mask overlap between halfwords 0x80145AA4 and 0x80145AB6 charges work byte
 * 0x0B toward 64 and starts a 30-tick hold at 64. Without overlap, a nonzero
 * byte decays by 1, 2 or 3 for ranges 1..32, 33..48 and 49..255. Work byte 2
 * selects half-rate decay, with its odd remainder applied on odd values of
 * word 0x80143E6C. Finally clamps the signed byte to 0..64. The work pointer
 * is scratchpad slot 0x44. Takes no arguments and returns nothing.
 * @status partial
 * @match 72.92
 * @residual Missing 8-byte frame; decay-mask scheduling, clamp registers and
 * branches differ; clamp zero store is split rather than shared.
 */
void updateMaskChargeAndDecayWorkByteBWorld03Area12114_801F5798(void) {
  u32 decay;
  s32 value;
  s8* work;

  if (area12114WorkCursor_1F800044[0x0A] != 0) {
    area12114WorkCursor_1F800044[0x0A]--;
  }
  if (area12114WorkCursor_1F800044[0x0A] == 0) {
    if ((D_80145AA4 & D_80145AB6) != 0) {
      if (area12114WorkCursor_1F800044[0x0B] < 64) {
        area12114WorkCursor_1F800044[0x0B]++;
      }
      if (area12114WorkCursor_1F800044[0x0B] == 64) {
        area12114WorkCursor_1F800044[0x0A] = 30;
      }
    } else if (area12114WorkCursor_1F800044[0x0B] != 0) {
      if (area12114WorkCursor_1F800044[0x0B] > 48) {
        decay = 3;
      } else if (area12114WorkCursor_1F800044[0x0B] > 32) {
        decay = 2;
      } else {
        decay = 1;
      }
      if (area12114WorkCursor_1F800044[2] != 0) {
        area12114WorkCursor_1F800044[0x0B] -= decay >> 1;
        if (D_80143E6C & 1) {
          area12114WorkCursor_1F800044[0x0B] -= decay & 1;
        }
      } else {
        area12114WorkCursor_1F800044[0x0B] -= decay;
      }
    }
    work = (s8*)area12114WorkCursor_1F800044;
    value = work[0x0B];
    work[0x0B] = value >= 0 ? (value < 65 ? value : 64) : 0;
  }
}
