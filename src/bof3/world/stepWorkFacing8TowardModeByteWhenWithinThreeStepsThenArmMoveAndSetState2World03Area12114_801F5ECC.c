#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area12114WorkCursor_1F800044;
/* @source 0x80145E98 @kind unknown */
extern u8 D_80145E98;

void advanceWord6cBy14AndSetState1WhenByte9ExpiresWorld03Area12114_801F5FF8(
    void);

/* @source 0x801F5ECC
 * @behavior Turns the facing byte 0x08 of the work record published at the
 * scratchpad pointer cell 0x1F800044 one step toward the shared mode byte at
 * 0x80145E98. The two arguments are that facing and that mode. Both are masked
 * to their low byte and compared as a magnitude; a magnitude below three turns
 * directly, and otherwise the shorter arc is tried by lifting the smaller of
 * the two by eight and re-comparing the pair, so a facing of 0 turns to 7 and a
 * facing of 7 turns to 0. A magnitude of three or more on both arcs, or an
 * exact match, returns zero without touching memory. Otherwise the turn stores
 * -0x40 into the step word at record offset 0x14 when the mode lies below the
 * facing and 0x40 otherwise, stores the stepped facing at offset 0x08, masks
 * that byte to three bits, republishes it at 0x80145E98, rearms the countdown
 * byte at offset 0x09 with 8, runs the turn-step update at 0x801F5FF8, sets the
 * state byte at offset 0x01 to 2 and returns one. The pointer cell is read
 * again before each of those writes, matching the original's per-use reloads.
 * @status partial
 * @match 45.33
 * @residual The first difference is the saved return-address store in place of
 * the original entry copy to a2. The mode copy survives in the first bgez delay
 * slot, but the facing copy is absent. Mask and work-pointer registers differ,
 * branch and jump targets shift, and the body is eight bytes short.
 * Reusing facing for the second magnitude and mode for the final masked value
 * keeps real calculation phases separate without extra memory accesses.
 */
s32 stepWorkFacing8TowardModeByteWhenWithinThreeStepsThenArmMoveAndSetState2World03Area12114_801F5ECC(
    s32 facing, s32 mode) {
  s32 turn;
  s32 target;
  u32 face;
  u32 mode8;
  s32 distance;
  u8* work;
  u8 next;

  turn = facing;
  target = mode;
  face = turn & 0xFF;
  mode8 = target & 0xFF;
  distance = face - mode8;
  if (distance < 0) {
    distance = -distance;
  }
  if (distance >= 3) {
    if (face < mode8) {
      turn = facing + 8;
    } else {
      target = mode + 8;
    }
    facing = (turn & 0xFF) - (target & 0xFF);
    if (facing < 0) {
      facing = -facing;
    }
    if (facing >= 3) {
      return 0;
    }
  }
  face = turn & 0xFF;
  mode = target & 0xFF;
  if (face != (u32)mode) {
    if ((u32)mode < face) {
      work = area12114WorkCursor_1F800044;
      next = work[8];
      FIELD_REF(s32, work, 0x14) = -0x40;
      next--;
    } else {
      work = area12114WorkCursor_1F800044;
      next = work[8];
      FIELD_REF(s32, work, 0x14) = 0x40;
      next++;
    }
    work[8] = next;
    area12114WorkCursor_1F800044[8] &= 7;
    D_80145E98 = area12114WorkCursor_1F800044[8];
    area12114WorkCursor_1F800044[9] = 8;
    advanceWord6cBy14AndSetState1WhenByte9ExpiresWorld03Area12114_801F5FF8();
    area12114WorkCursor_1F800044[1] = 2;
    return 1;
  }
  return 0;
}
