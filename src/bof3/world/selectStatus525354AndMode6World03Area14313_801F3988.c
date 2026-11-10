#include "bof3/bof3.h"

/* @source 0x801490C7 @kind data */
extern s8 D_801490C7;
/* @source 0x80145548 @kind data */
extern u32 D_80145548;
/* @source 0x801490A8 @kind data */
extern u16 D_801490A8;
/* @source 0x801D4285 @kind data */
extern u8 D_801D4285;

/* @source 0x801F3988
 * @behavior Stores status 0x54 when the shared gate byte is nonzero, or 0x53
 * when the gate is zero and D_80145548 differs from 0x3FFFF. Both paths also
 * store 6 into the shared mode byte. Otherwise stores status 0x52 without
 * changing the mode byte. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectStatus525354AndMode6World03Area14313_801F3988(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x54;
    D_801D4285 = 6;
  } else if (D_80145548 == 0x3FFFF) {
    D_801490A8 = 0x52;
  } else {
    D_801490A8 = 0x53;
    D_801D4285 = 6;
  }
}
