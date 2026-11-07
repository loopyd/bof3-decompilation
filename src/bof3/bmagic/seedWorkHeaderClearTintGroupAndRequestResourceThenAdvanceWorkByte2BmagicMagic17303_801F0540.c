#include "bof3/bof3.h"

/* Resource request wrapper of the main executable; the sibling bmagic lift
 * seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500_801EF4A4
 * declares the same prototype and passes a resource id in the same slot. It is
 * the only reference this span makes outside its own bytes and the scratchpad
 * work object, and it is still named by the remaining asm of this target, so
 * the generated undefined_funcs_auto.txt binds it. */
extern void func_8014D6B8(u32 flag);

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is the form this span encodes before
 * each store group; the fixed-address lvalue form of the same cell kept the
 * cell address in a callee-saved register across the helper call instead. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801F0540
 * @behavior Overlay entry seed of this payload: takes no arguments, returns
 *           nothing and writes no cell outside the scratchpad work object.
 *           Every store group re-reads the work-object pointer cell at
 *           0x1F800044 through its own local, because a store through the
 *           loaded pointer may alias the cell itself, which is why the original
 *           reloads the cell before each group instead of keeping the pointer.
 *           The header group writes byte 0x25 = 0x1D, byte 0x26 = 0, byte 0x27 =
 *           0xA0, byte 0x28 = 0 and byte 0x24 = 4 in that order. It then clears
 *           the signed tint triplet at bytes 0x5D, 0x5E and 0x5F before the mode
 *           byte 0x5C, reading the cell again for each of the four stores. Byte
 *           0x2A = 0 and byte 0x29 = 3 follow, then one load feeds both the
 *           halfword at 0x2C = 0 and byte 0x2B = 1, the second of which the
 *           original keeps in the jal delay slot. The function requests resource
 *           0 through the resident wrapper func_8014D6B8, clears work byte 9 and
 *           advances work byte 2 with a byte-width read, increment and store. The
 *           frame is 0x18 bytes with the return address at 0x10 and no saved
 *           registers, and no value is returned.
 *
 *           No byte-identical sibling exists: a search of every payload under
 *           out/binaries for this 0x108-byte span finds no other copy, so the
 *           clone lever is declined. The body, statement order, one-local-per-
 *           store-group form and cell reload discipline come from the exact
 *           sibling at 0x801EF4A4 in the magic045 #0 payload, whose own entry
 *           seed differs only in its stored literals, in the battle record copy
 *           it makes before the first store group and in the byte-0 flag update
 *           it performs; this function writes its own literals and omits those
 *           two groups.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkHeaderClearTintGroupAndRequestResourceThenAdvanceWorkByte2BmagicMagic17303_801F0540(
    void) {
  u8 *work;
  u8 *next;
  u8 *tilt;
  u8 *offset;
  u8 *kind;
  u8 *tintR;
  u8 *tintG;
  u8 *tintB;
  u8 *mode;
  u8 *tail;
  u8 *index;
  u8 *pair;
  u8 *clear;
  u8 *step;

  work = D_1F800044;
  work[0x25] = 0x1D;
  next = D_1F800044;
  next[0x26] = 0;
  tilt = D_1F800044;
  tilt[0x27] = 0xA0;
  offset = D_1F800044;
  offset[0x28] = 0;
  kind = D_1F800044;
  kind[0x24] = 4;
  tintR = D_1F800044;
  *(s8 *)(tintR + 0x5D) = 0;
  tintG = D_1F800044;
  *(s8 *)(tintG + 0x5E) = 0;
  tintB = D_1F800044;
  *(s8 *)(tintB + 0x5F) = 0;
  mode = D_1F800044;
  mode[0x5C] = 0;
  tail = D_1F800044;
  tail[0x2A] = 0;
  index = D_1F800044;
  index[0x29] = 3;
  pair = D_1F800044;
  *(u16 *)(pair + 0x2C) = 0;
  pair[0x2B] = 1;
  func_8014D6B8(0);
  clear = D_1F800044;
  clear[9] = 0;
  step = D_1F800044;
  step[2]++;
}
