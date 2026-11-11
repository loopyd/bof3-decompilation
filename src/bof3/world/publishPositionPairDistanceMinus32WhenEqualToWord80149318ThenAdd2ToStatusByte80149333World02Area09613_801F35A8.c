#include "bof3/bof3.h"

/* Shared position-coordinate words of the main executable, published by the
 * front-end selectors and projected by the world overlays. Both lie outside
 * this overlay's payload; Splat binds them through this target's generated
 * undefined-syms map, so no target-map row is needed. */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* Distance helper of the shared main executable: it takes two signed 16.16
 * coordinate words and answers their distance as a halfword. The original adds
 * 0x100 to that answer with a plain 32-bit add and no widening or masking, so
 * this overlay sees a word return and narrows it at the assignment. */
extern s32 func_8015477C(s32 x, s32 y);

/* Shared distance word this handler publishes to. It lies outside this
 * overlay's payload and is covered by the same generated map.
 * @source 0x80149318 @kind unknown
 */
extern u32 D_80149318;

/* Shared area status byte this handler advances, also outside the payload.
 * @source 0x80149333 @kind unknown
 */
extern u8 D_80149333;

/* @source 0x801F35A8
 * @behavior Takes no arguments and returns nothing. It passes the two shared
 *           position-coordinate words 0x80145EC4 and 0x80145EC8 to the shared
 *           distance helper 0x8015477C, which is called before the frame is
 *           created because both argument loads are plain fixed-address loads,
 *           and narrows that word answer once, at the assignment, to a signed
 *           halfword widened back to a word so the sign extension is
 *           materialised in the answer register itself. When that distance
 *           equals the value the shared word 0x80149318 currently holds, it
 *           subtracts 0x20 from the distance so the published value differs
 *           from the one already there; every other distance is stored
 *           unchanged. It then writes the distance to 0x80149318 and re-reads
 *           the shared area status byte 0x80149333 into that same consumed
 *           distance local, storing the byte plus 2 back into it. The frame is
 *           0x18 bytes and holds only the return address at offset 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishPositionPairDistanceMinus32WhenEqualToWord80149318ThenAdd2ToStatusByte80149333World02Area09613_801F35A8(
    void) {
  s32 distance;

  distance = (s16)func_8015477C(D_80145EC4, D_80145EC8);
  if (D_80149318 == distance) {
    distance = distance - 0x20;
  }
  D_80149318 = distance;
  distance = D_80149333;
  D_80149333 = distance + 2;
}
