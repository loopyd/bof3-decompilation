#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* D_1F800044;
/* @source 0x80145EC4 @kind unknown */
extern u32 D_80145EC4;
/* @source 0x80145EC8 @kind unknown */
extern u32 D_80145EC8;
/* @source 0x801F57EC @kind table */
extern u16 D_801F57EC[][2];
/* @source 0x801F57FC @kind table */
extern u8 D_801F57FC[][2];

void func_8014DD3C(u16 entry);
void func_8014D6B8(u32 resource);
void func_8014D290(void);

/* @source 0x801F4328
 * @behavior Entry 1 of the three-entry handler table 0x801F57E0 for the work
 *           record published at the scratchpad pointer cell 0x1F800044, the
 *           table the overlay step dispatch 0x801F41A8 indexes with work byte
 *           0x01. It applies main lookup entry 0x46 through the resident helper
 *           0x8014DD3C before clearing work bytes 0x48 and 0x24 and storing 5
 *           in work byte 0x29, then seeds the step word at 0x0C and the step
 *           word at 0x10 with the signed halfword pair that the facing byte
 *           0x08 selects, at four bytes per pair, from the four-pair table
 *           0x801F57EC, and copies the shared position words 0x80145EC4 and
 *           0x80145EC8 into record offsets 0x34 and 0x38. While work byte 0x06
 *           is set it subtracts the signed thirteenth-unit of that pair from
 *           the position halfwords 0x36 and 0x3A and, when the seeded word at
 *           0x0C reads zero, steps halfword 0x36 by 2 in the direction work
 *           byte 0x06 selects, otherwise storing the stepped halfword 0x3A
 *           moved by 2. It then subtracts the signed ninth-unit of the same
 *           pair from both halfwords, clears work byte 0x0B, increments work
 *           byte 0x01 so the next step dispatches handler 2, requests the first
 *           byte of the pair 0x801F57FC that the facing byte selects through
 *           the shared helper 0x8014D6B8, stores that pair second byte in work
 *           byte 0x2A and finally calls the shared helper 0x8014D290. Takes no
 *           arguments, saves only the return address at 0x10 of its 0x18-byte
 *           frame and returns nothing. Its 0x230 original bytes differ from the
 *           lifted
 *           seedFacingStepPairAndStepPositionThenAdvanceModeAndRequestResourceWorld01Area06513_801F4330
 *           in emi/world01/area065/13 at ten words, measured against the two
 *           payloads under out/binaries: the eight halfword-offset immediates of
 *           the local tables 0x801F57EC and 0x801F57FC, each lower there by
 *           0x4C0, and the two internal jump words to each span's own epilogue,
 *           each lower there by the 2 that encodes the +8 the two spans sit
 *           apart. The four exact
 *           seedFacingStepPairAndStepPositionThenAdvanceModeAndRequestResource
 *           lifts in emi/world00/area033/13, emi/world01/area065/13,
 *           emi/world03/area121/14 and emi/world03/area151/13 carry the same
 *           body, so this source takes that family statement order with this
 *           target's own table and helper bindings.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedFacingStepPairAndStepPositionThenAdvanceModeAndRequestResourceWorld02Area08713_801F4328(
    void) {
  u8* record;
  s16 stepped;

  func_8014DD3C(0x46);
  D_1F800044[0x48] = 0;
  D_1F800044[0x24] = 0;
  D_1F800044[0x29] = 5;

  record = D_1F800044;
  FIELD_REF(s32, record, 0x0C) = (s16)D_801F57EC[record[0x08]][0];
  FIELD_REF(s32, record, 0x10) = (s16)D_801F57EC[record[0x08]][1];
  FIELD_REF(u32, record, 0x34) = D_80145EC4;
  FIELD_REF(u32, record, 0x38) = D_80145EC8;

  if (record[0x06] != 0) {
    FIELD_REF(u16, record, 0x36) -= (s16)D_801F57EC[record[0x08]][0] >> 13;
    stepped =
        FIELD_REF(u16, record, 0x3A) - ((s16)D_801F57EC[record[0x08]][1] >> 13);
    FIELD_REF(u16, record, 0x3A) = stepped;
    if (record[0x06] == 1) {
      if (FIELD_REF(s32, record, 0x0C) == 0) {
        FIELD_REF(u16, record, 0x36) += 2;
      } else {
        FIELD_REF(u16, record, 0x3A) = stepped + 2;
      }
    } else if (FIELD_REF(s32, record, 0x0C) == 0) {
      FIELD_REF(u16, record, 0x36) -= 2;
    } else {
      FIELD_REF(u16, record, 0x3A) = stepped - 2;
    }
  }

  {
    u8* next;
    u16 half;

    record = D_1F800044;
    FIELD_REF(u16, record, 0x36) -= (s16)D_801F57EC[record[0x08]][0] >> 9;
    half = FIELD_REF(u16, record, 0x3A) - ((s16)D_801F57EC[record[0x08]][1] >> 9);
    record[0x0B] = 0;
    next = D_1F800044;
    FIELD_REF(u16, record, 0x3A) = half;
    next[0x01]++;
  }
  func_8014D6B8(D_801F57FC[D_1F800044[0x08]][0]);
  D_1F800044[0x2A] = D_801F57FC[D_1F800044[0x08]][1];
  func_8014D290();
}
