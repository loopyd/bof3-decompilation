#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Every access re-reads this cell,
 * because a byte store through the record may alias it. */
extern u8* D_1F800044;

/* Four-entry byte table of this overlay: the countdown values selected by the
 * two low bits of the work record's position halfwords +0x36 and +0x3A, read as
 * 0x73, 0x4B, 0x5A, 0x3C in the shipped payload.
 * @source 0x801F395C @kind table
 */
extern u8 D_801F395C[];

/* Distance helper of the shared main executable: it takes a signed 16.16 word
 * pair as its two coordinates and answers their distance as a halfword. This
 * overlay uses the answer both as a stored halfword and as a signed value the
 * comparison sign-extends, so the full returned word is kept. */
extern s32 func_8015477C(s32 x, s32 y);

/* @source 0x801F32A8
 * @behavior Entry 0 of the state-byte-+0x02 dispatch table at 0x801F3948 that
 *           the dispatcher 0x801F3264 indexes: reads the work record published
 *           by the scratchpad pointer cell 0x1F800044 and forms a two-bit index
 *           from the low bit of record halfwords +0x36 and +0x3A, the second
 *           bit shifted up by one. It stores the byte at that index of the
 *           four-entry table 0x801F395C into record byte +0x0A and then into
 *           record byte +0x09, the saved-byte and countdown pair this overlay's
 *           neighbours use. It then hands record words +0x34 and +0x38, the
 *           16.16 position pair, to the shared distance helper func_8015477C
 *           twice: the first answer is stored as the halfword +0x3E, and the
 *           second call takes the same pair with its first word advanced and
 *           its second word reduced by 0x8000, that is half a unit in opposing
 *           directions, and its answer is taken as a signed halfword. When the
 *           stored halfword +0x3E is smaller than that answer the body stores 0
 *           in record word +0x0C and 1 in record word +0x10, otherwise 1 in
 *           +0x0C and 0 in +0x10, so the pair is the complement of that test and
 *           the signed answer itself is consumed by the comparison alone. It
 *           finally increments the record's dispatch state byte +0x02. The
 *           record pointer is re-read once per access group, as the original
 *           does. Takes no arguments and returns nothing; the 0x18-byte frame
 *           holds only the saved return address at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectCountdownFromPositionHalfwordLsbsStoreDistanceAndOffsetCompareThenAdvanceStateWorld02Area10213_801F32A8(
    void) {
  u8* level;
  u8* from;
  u8* position;
  u8* gate;
  u8* state;
  u8 countdown;
  s32 distance;
  s16 offsetDistance;

  level = D_1F800044;
  countdown = D_801F395C[((*(u16*)(level + 0x36) & 1) |
                          ((*(u16*)(level + 0x3A) & 1) << 1))];
  level[0xA] = countdown;
  level[9] = countdown;
  from = D_1F800044;
  distance = func_8015477C(*(s32*)(from + 0x34), *(s32*)(from + 0x38));
  position = D_1F800044;
  *(s16*)(position + 0x3E) = distance;
  offsetDistance = func_8015477C(*(s32*)(position + 0x34) + 0x8000,
                                 *(s32*)(position + 0x38) - 0x8000);
  gate = D_1F800044;
  if (*(s16*)(gate + 0x3E) < offsetDistance) {
    *(s32*)(gate + 0xC) = 0;
    *(s32*)(gate + 0x10) = 1;
  } else {
    *(s32*)(gate + 0xC) = 1;
    *(s32*)(gate + 0x10) = 0;
  }
  state = D_1F800044;
  state[2]++;
}
