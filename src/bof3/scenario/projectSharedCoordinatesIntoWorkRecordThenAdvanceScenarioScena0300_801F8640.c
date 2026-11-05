#include "bof3/bof3.h"

extern u8* D_1F800044;
extern s16 D_1F800014;
extern s16 D_1F800016;
extern s16 D_1F800018;
extern u16 D_1F800034;
extern u16 D_1F800036;
extern s32 D_80145EC4;
extern s32 D_80145EC8;
extern s16 D_80145ECE;

/* @source 0x801F8640
 * @behavior Projects the shared camera position and azimuth words at
 * 0x80145EC4, 0x80145EC8 and 0x80145ECE into the scratchpad vertex at
 * 0x1F800014 (x and y shifted right by 9 and biased by -0x4000, z the
 * negated signed halfword halved), passes that vertex with the scratchpad
 * screen pair at 0x1F800034 to RotTransPers, then writes the returned
 * ordering value into work word 0x60, the projected screen pair into work
 * halfwords 0x2E and 0x30, clears work byte 9 and increments work byte 1
 * through a second read of the work pointer cell at 0x1F800044. Takes no
 * arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectSharedCoordinatesIntoWorkRecordThenAdvanceScenarioScena0300_801F8640(
    void) {
  SVECTOR* vtx;
  long* sxy;
  long depth;
  long flag;
  s32 otz;
  u16 screen_y;
  u8* record;
  u8* cursor;

  vtx = (SVECTOR*)&D_1F800014;
  sxy = (long*)&D_1F800034;

  vtx->vx = (s16)(D_80145EC4 >> 9) - 0x4000;
  D_1F800016 = (s16)(D_80145EC8 >> 9) - 0x4000;
  D_1F800018 = (s16)(-D_80145ECE / 2);

  otz = RotTransPers(vtx, sxy, &depth, &flag);
  record = D_1F800044;
  *(s32*)(record + 0x60) = otz;
  *(u16*)(record + 0x2e) = *(u16*)sxy;
  screen_y = D_1F800036;
  *(u8*)(record + 0x09) = 0;
  cursor = D_1F800044;
  *(u16*)(record + 0x30) = screen_y;
  cursor[1]++;
}
