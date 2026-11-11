#include "bof3/bof3.h"

extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* @source 0x801F31BC @kind function */
extern void func_801F31BC(void);

/* @source 0x801F303C
 * @behavior Reads the scratchpad work record at 0x1F800044. Route byte +8
 *           selects the camera coordinate and signed record halfword: nonzero
 *           uses D_80145EC8 and +0x3A, zero uses D_80145EC4 and +0x36.
 *           Advances work byte +2 if the absolute distance from the selected
 *           cell centre (((halfword + 1) << 16) | 0x8000) exceeds 0x20000,
 *           or if the other coordinate is more than 0x20000 from both cell
 *           starts (halfword << 16) and ((halfword + 1) << 16). Calls local
 *           helper 0x801F31BC on every path. Takes no arguments and returns
 *           nothing. The shared coordinate words are outside this payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte2WhenCameraOutsideRecordCoordinateWindowWorld04Area16713_801F303C(
    void) {
  u8* work;
  s32 value;
  s32 base;
  s32 limit;

  work = SPAD_ADDR(u8, 0x44u);
  work = *(u8**)work;
  limit = 0x20000;

  if (work[8] != 0) {
    s32 d;

    d = D_80145EC8 - (((FIELD_REF(s16, work, 0x3A) + 1) << 16) | 0x8000);
    if (d < 0) {
      d = -d;
    }
    d = d > limit;
    if (d) {
      goto advance;
    }
    value = FIELD_REF(s16, work, 0x36);
    base = D_80145EC4;
  } else {
    s32 d;

    d = D_80145EC4 - (((FIELD_REF(s16, work, 0x36) + 1) << 16) | 0x8000);
    if (d < 0) {
      d = -d;
    }
    d = d > limit;
    if (d) {
      goto advance;
    }
    value = FIELD_REF(s16, work, 0x3A);
    base = D_80145EC8;
  }

  {
    s32 d;

    d = base - (value << 16);
    if (d < 0) {
      d = -d;
    }
    d = d > limit;
    if (d) {
      d = base - ((value + 1) << 16);
      if (d < 0) {
        d = -d;
      }
      d = d > limit;
      if (d) {
advance:
        work[2]++;
      }
    }
  }

  func_801F31BC();
}
