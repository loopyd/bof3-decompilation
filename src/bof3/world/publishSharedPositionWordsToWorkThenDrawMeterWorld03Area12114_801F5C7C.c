#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area12114WorkCursor_1F800044;
/* @source 0x80145EC4 @kind unknown */
extern u32 D_80145EC4;
/* @source 0x80145EC8 @kind unknown */
extern u32 D_80145EC8;
/* @source 0x80145ECE @kind unknown */
extern u16 D_80145ECE;

void func_8015A944(void);
void drawShadedMeterFrameAndDepletingFillWorld03Area12114_801F5B90(void);

/* @source 0x801F5C7C
 * @behavior Per-frame position publish for the work record at the scratchpad
 * pointer cell 0x1F800044, run directly by the turn-step update at 0x801F5FF8
 * and from the settle helper at 0x801F5E2C. Copies the shared position words
 * 0x80145EC4 and 0x80145EC8 into record offsets 0x34 and 0x38 and the shared
 * halfword 0x80145ECE into record offset 0x3E, calls the resident helper at
 * 0x8015A944 with whatever argument registers the caller left, and then runs the
 * overlay meter draw at 0x801F5B90. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishSharedPositionWordsToWorkThenDrawMeterWorld03Area12114_801F5C7C(
    void) {
  u8* work;

  work = area12114WorkCursor_1F800044;
  FIELD_REF(u32, work, 0x34) = D_80145EC4;
  FIELD_REF(u32, work, 0x38) = D_80145EC8;
  FIELD_REF(u16, work, 0x3E) = D_80145ECE;
  func_8015A944();
  drawShadedMeterFrameAndDepletingFillWorld03Area12114_801F5B90();
}
