#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80146875;

void drawTexturedQuad64x32Clut1ebWorld04Area17213_801F4E8C(s32 arg0, s32 arg1);
void drawSlantedSubtractiveGouraudBandWorld04Area17213_801F4F70(s32 arg0,
    s32 arg1);

/* @source 0x801F4DE0
 * @behavior Advances the work record published through the scratchpad pointer
 * cell 0x1F800044: adds 0x0A to its halfword at offset 0x2E, stores the
 * truncated halfword, and once that stored value reads at least 0x191 as a
 * signed halfword increments the shared counter byte at 0x80146875 and writes
 * 2 into the record's state byte at offset 0x01. It then runs the overlay draw
 * routine at 0x801F4E8C with 0xDC and 0xC8 and, after a fresh load of the cell,
 * draws the record halfword at 0x2E with height 0xC8 through 0x801F4F70. Takes
 * no arguments and returns nothing; the 0x18-byte frame only keeps $ra at 0x10
 * across the calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword2eBy0aAndSetState1To2At191ThenDrawHalfword2eWorld04Area17213_801F4DE0(
    void) {
  u8* work;
  u8* next;
  s16* half;

  work = D_1F800044;
  half = (s16*)(work + 0x2e);
  *half = (s16)(*half + 0x0a);
  if (*half >= 0x191) {
    u8* counter;

    counter = &D_80146875;
    *counter = *counter + 1;
    work[1] = 2;
  }
  drawTexturedQuad64x32Clut1ebWorld04Area17213_801F4E8C(0xdc, 0xc8);
  next = D_1F800044;
  drawSlantedSubtractiveGouraudBandWorld04Area17213_801F4F70(
      *(s16*)(next + 0x2e), 0xc8);
}
