#include "bof3/bof3.h"

#include <stdlib.h>

extern u8 D_80144ED0; /* @source 0x80144ED0 @kind data */
extern u32 D_8014931C; /* @source 0x8014931C @kind data */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */
extern u8* world03Area11913WorkRecord; /* @source 0x1F800044 @kind data */
extern s32 func_8015B5D4(u32 bank, s32 index); /* @source 0x8015B5D4 @kind function */
extern void func_801F3324(void); /* @source 0x801F3324 @kind function */

/* @source 0x801F2EDC
 * @behavior Clears work halfword 0x30 and probes flag 0x0F in bank 0x80144ED0.
 *           When clear, stamps 0x52 into map cells at offsets 0x41 plus 62
 *           and 63 times the shared row stride, and sets work byte 2 to 5.
 *           Otherwise increments work byte 2; when the camera is within
 *           0x8000 of 0x428000 on its first coordinate and within 0x10000
 *           of either 0x3E0000 or 0x3F0000 on its second, sets work halfword
 *           0x30 to -0x100 and byte 2 to 3. Every path calls the overlay
 *           draw routine at 0x801F3324. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkHalfword30AndRouteStateByFlag0FAndCameraThenDrawWorld03Area11913_801F2EDC(void) {
  u8* stride;
  u8* work;

  FIELD_REF(u16, world03Area11913WorkRecord, 0x30) = 0;
  if (func_8015B5D4((u32)&D_80144ED0, 0x0F) == 0) {
    stride = PSX_PTR(u8, 0x80104000u);
    *(u8*)(D_8014931C + stride[0] * 62 + 0x41) = 0x52;
    *(u8*)(D_8014931C + stride[0] * 63 + 0x41) = 0x52;
    world03Area11913WorkRecord[2] = 5;
  } else {
    world03Area11913WorkRecord[2]++;
    if (abs(D_80145EC4 - 0x428000) <= 0x8000 &&
        (abs(D_80145EC8 - 0x3E0000) <= 0x10000 ||
         abs(D_80145EC8 - 0x3F0000) <= 0x10000)) {
      work = world03Area11913WorkRecord;
      FIELD_REF(s16, work, 0x30) = -0x100;
      work[2] = 3;
    }
  }
  func_801F3324();
}
