#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through the encoded symbol name, so no target-map row or
 * WEAK_SYMBOL_AT binding is needed. The sibling bmagic lifts read the same
 * record through this pointer (copyWorkWordTripletAndClearByte9ThenAdvance
 * WorkByte2BmagicMagic04103_801EF484 copies its word triplet, incrementWork
 * Byte2WhenGlobalWorkByteBIs84BmagicMagic11303_801EF8A4 tests its byte 0xB). */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, and the compiler re-reads the cell after a
 * store that may alias it, which is what the original does before each store
 * group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Resource request helper of the main executable; the sibling world and ui
 * lifts declare the same prototype and pass a resource id. */
extern void func_8014D6B8(u32 flag);

/* @source 0x801EF4A4
 * @behavior Overlay entry seed: reads the shared battle work record through
 *           0x801EB4E0 and copies its words at 0x34 and 0x38 and its halfword
 *           at 0x3E into the scratchpad work object published at the pointer
 *           cell 0x1F800044, then seeds that object: byte 0x24 = 4, byte 0x25 =
 *           0x1D, byte 0x26 = 0, byte 0x27 = 0xA0, byte 0x28 = 0, byte 0x29 =
 *           4, byte 0x2A = 0, byte 0x2B = 0, the halfword at 0x2C = 0, the
 *           16.16 motion pair at 0x40/0x44 = 0x10000, byte 0x48 = 1, mode byte
 *           0x5C = 2, the signed tint triplet at 0x5D-0x5F = -0x40, and bit
 *           0x20 of byte 0. It then requests resource 0 through func_8014D6B8,
 *           clears work byte 9 and advances work byte 2 with a byte-width add.
 *           Each store group reads the pointer cell through its own local: the
 *           original reloads the cell before every group because a store through
 *           the loaded pointer may alias the cell, and one local reused for all
 *           groups collapses those loads onto a single register. The entry copy
 *           and the 0x3E halfword store share the first load, and the 0x2C/0x2B
 *           pair shares the load made before the helper call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500_801EF4A4(void) {
  u8 *source;
  u8 *work;
  u8 *next;
  u8 *motion;
  u8 *tilt;
  u8 *offset;
  u8 *kind;
  u8 *flags;
  u8 *mode;
  u8 *tintR;
  u8 *tintG;
  u8 *tintB;
  u8 *tail;
  u8 *index;
  u8 *pair;
  u8 *clear;
  u8 *step;
  u16 distance;

  source = D_801EB4E0;
  work = D_1F800044;
  *(u32 *)(work + 0x34) = *(u32 *)(source + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(source + 0x38);
  distance = *(u16 *)(source + 0x3E);
  work[0x25] = 0x1D;
  next = D_1F800044;
  *(u16 *)(work + 0x3E) = distance;
  next[0x26] = 0;
  motion = D_1F800044;
  *(u32 *)(motion + 0x40) = 0x10000u;
  *(u32 *)(motion + 0x44) = 0x10000u;
  motion[0x48] = 1;
  tilt = D_1F800044;
  tilt[0x27] = 0xA0;
  offset = D_1F800044;
  offset[0x28] = 0;
  kind = D_1F800044;
  kind[0x24] = 4;
  flags = D_1F800044;
  flags[0] |= 0x20;
  mode = D_1F800044;
  mode[0x5C] = 2;
  tintR = D_1F800044;
  *(s8 *)(tintR + 0x5D) = -0x40;
  tintG = D_1F800044;
  *(s8 *)(tintG + 0x5E) = -0x40;
  tintB = D_1F800044;
  *(s8 *)(tintB + 0x5F) = -0x40;
  tail = D_1F800044;
  tail[0x2A] = 0;
  index = D_1F800044;
  index[0x29] = 4;
  pair = D_1F800044;
  *(u16 *)(pair + 0x2C) = 0;
  pair[0x2B] = 0;
  func_8014D6B8(0);
  clear = D_1F800044;
  clear[9] = 0;
  step = D_1F800044;
  step[2]++;
}
