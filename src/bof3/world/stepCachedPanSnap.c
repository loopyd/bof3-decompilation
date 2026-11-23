#include "bof3/bof3.h"

extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind unknown */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind unknown */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */
extern u16 D_801F4E04; /* @source 0x801F4E04 @kind unknown */
extern u8 D_801F4E10; /* @source 0x801F4E10 @kind unknown */
extern u16 D_801F4E14; /* @source 0x801F4E14 @kind unknown */

/* @source 0x801F3304
 * @behavior Overlay state handler at index 17 of the local handler table at
 *           0x801F3FF4 and index 13 of the table at 0x801F4004, both selected
 *           with the work record byte at +1, so it runs while that byte reads
 *           17 or 13. It advances this overlay's own pan-frame byte
 *           D_801F4E10 by one and branches on the truncated new value.
 *           Below 8 it repeats the pan in the direction cached in the
 *           overlay's halfword D_801F4E14 by the pan entry handler
 *           stepCoordinatePairByFaceButtonAndArmState12World01Area06813 at
 *           0x801F30B0: for each of the eight face-button or diagonal-pair
 *           values it republishes the first 0x140-byte work record D_80145E90
 *           as both the shared record pointer D_80146250 and the scratchpad
 *           work cursor at 0x1F800044, steps the party coordinate pair by
 *           0x800 in the cached direction - 0x1000/0x3000 add 0x800 to
 *           D_80145EC8, 0x4000/0xC000 subtract it, 0x2000/0x6000 subtract
 *           0x800 from D_80145EC4 and 0x8000/0x9000 add it, every direction
 *           inverted from that entry handler - restores the previously
 *           published cursor, and finally arms the record byte at +0x01 with
 *           0xB, index 11 of the 0x801F4004 table and index 15 of the
 *           0x801F3FF4 table, both the pan entry handler above. Once the
 *           frame byte reaches 8 it instead masks the low twelve bits off both
 *           party coordinates, rounds them down to a 0x1000 boundary, clears
 *           the pad-match counter D_801F4E04 and the frame byte D_801F4E10
 *           and arms that record byte with 0xF, index 15 of the 0x801F4004
 *           table and index 19 of the 0x801F3FF4 table, the pad-count step at
 *           0x801F3538 - and returns without the pan store. Leaf with no
 *           frame: it takes no arguments, makes no call and returns nothing.
 *           Shape evidence, no clone: the mirror sibling above holds the same
 *           eight-case dispatch, publish, restore and post-switch state store
 *           with every 0x800 direction inverted, so its statement order,
 *           operand forms and scratchpad cell spelling are taken from it while
 *           the directions, the 0xB/0xF state constants and the 0x800 frame
 *           budget are written from these bytes; no payload among the 396
 *           extracted EMI binaries carries this span or its leading
 *           `lui`/`lbu` pair, which encodes this overlay's own cell 0x801F4E10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepCachedPanSnap(void) {
  u8** slots;
  u8* previous;
  u8 frame;

  frame = D_801F4E10 + 1;
  D_801F4E10 = frame;
  if (frame >= 8) {
    slots = SPAD_PTR_TABLE(u8);
    previous = slots[0x11];
    D_80146250 = D_80145E90;
    slots[0x11] = D_80145E90;
    D_80145EC8 &= -0x1000;
    D_80145EC4 &= -0x1000;
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11] = previous;
    previous[0x01] = 0xF;
    D_801F4E04 = 0;
    D_801F4E10 = 0;
  } else {
    switch (D_801F4E14) {
    case 0x1000:
    case 0x3000:
      slots = SPAD_PTR_TABLE(u8);
      previous = slots[0x11];
      D_80146250 = D_80145E90;
      slots[0x11] = D_80145E90;
      D_80145EC8 += 0x800;
      slots = SPAD_PTR_TABLE(u8);
      slots[0x11] = previous;
      break;
    case 0x2000:
    case 0x6000:
      slots = SPAD_PTR_TABLE(u8);
      previous = slots[0x11];
      D_80146250 = D_80145E90;
      slots[0x11] = D_80145E90;
      D_80145EC4 -= 0x800;
      slots = SPAD_PTR_TABLE(u8);
      slots[0x11] = previous;
      break;
    case 0x4000:
    case 0xC000:
      slots = SPAD_PTR_TABLE(u8);
      previous = slots[0x11];
      D_80146250 = D_80145E90;
      slots[0x11] = D_80145E90;
      D_80145EC8 -= 0x800;
      slots = SPAD_PTR_TABLE(u8);
      slots[0x11] = previous;
      break;
    case 0x8000:
    case 0x9000:
      slots = SPAD_PTR_TABLE(u8);
      previous = slots[0x11];
      D_80146250 = D_80145E90;
      slots[0x11] = D_80145E90;
      D_80145EC4 += 0x800;
      slots = SPAD_PTR_TABLE(u8);
      slots[0x11] = previous;
      break;
    }
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 0xB;
  }
}
