#include "bof3/bof3.h"
#include <rand.h>

extern u8* D_1F800044;

/* @source 0x801F8490
 * @behavior Entry 0 of the overlay work-object handler table at 0x801FCFA0,
 * which the frame dispatcher selects with byte 1 of the work object published at
 * scratchpad pointer slot 0x1F800044. It seeds the effect-rectangle record that
 * begins at work + 0x0C: signed halfword 0x0C (intensity) is 0x600, halfwords
 * 0x0E and 0x10 take rand() masked with 0x700 and 0xF00, halfwords 0x12 and
 * 0x14 (width and height) are 0x100, and colour bytes 0x18, 0x17 and 0x16 are
 * 0xFF; a set low bit of a further rand() then clears byte 0x16 and a set low
 * bit of another clears byte 0x18. Finally it reloads the scratchpad pointer and
 * advances work byte 1, so the next frame dispatches entry 1 (0x801F853C) of the
 * same table. Takes no arguments and returns nothing.
 * The record layout is the one the drawing helper 0x801F88FC reads from its
 * work + 0x0C argument: signed halfwords at +0x0 (intensity), +0x2, +0x4,
 * +0x6 (width) and +0x8 (height), and colour bytes at +0xA, +0xB and +0xC.
 * Offsets and the scratchpad spelling follow the sibling lift of the same
 * table, src/bof3/scenario/drawAndShrinkWorkHalfwordsThenAdvanceScenarioScena0300_801F853C.c,
 * which shrank halfwords 0x12 and 0x14 from the 0x100 seeded here, and which
 * shows the same tail shape: the reload after the colour stores has to land in
 * its own local, so the pointer is built in $v1 instead of surviving in $s0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedEffectRectWithRandomOffsetsAndAdvanceWorkByte1ScenarioScena0300_801F8490(
    void) {
  u8* work;
  u8* next_work;

  work = D_1F800044;
  FIELD_REF(s16, work, 0x0C) = 0x600;
  FIELD_REF(s16, work, 0x12) = 0x100;
  FIELD_REF(s16, work, 0x14) = 0x100;
  FIELD_REF(s16, work, 0x0E) = rand() & 0x700;
  FIELD_REF(s16, work, 0x10) = rand() & 0xF00;
  work[0x18] = 0xFF;
  work[0x17] = 0xFF;
  work[0x16] = 0xFF;
  if (rand() & 1) {
    work[0x16] = 0;
  }
  if (rand() & 1) {
    work[0x18] = 0;
  }
  next_work = D_1F800044;
  next_work[1]++;
}
