#include "bof3/battle/battle15_internal.h"

/* Readiness poll helper of the main executable, outside this payload. The
 * battle and bmagic lifts declare the same prototype and discard or test its
 * returned byte. */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC */

/* @source 0x800A8DE8
 * @behavior Loads the scratch work object published at the pointer cell
 *   0x1F800044 and raises bit 0x20 of that object's byte 0, sets object byte
 *   0x5C to 3, then writes the same value -(object byte 0xB << 4) to object
 *   bytes 0x5D, 0x5F and 0x5E in that store order, re-reading both the cell and
 *   byte 0xB for each of the three. It then sets object byte 0x9 to 8 and
 *   advances object byte 0x01 by one, and that store fills the delay slot of
 *   its single call, the readiness helper func_8014DAEC, whose returned byte
 *   this step discards. Takes no arguments and returns nothing, and reads or
 *   writes no other state. No jal in this payload names 0x800A8DE8; its only
 *   in-image reference is the code-pointer word at 0x800B50D4, which follows
 *   the 0x800A8CC8 and 0x800A8D3C words of that run and precedes the 0x800A8EB8
 *   word at 0x800B50D8. The roles of bytes 0x5C, 0x5D, 0x5E, 0x5F, 0x09 and
 *   0x0B are not established, so the name describes the stores.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseByte0Flag20SetTriplet5D5F5EToMinus16xByteBSetByte5CTo3Byte9To8AdvanceWorkByte1(
    void) {
  u8 *flag;
  u8 *stage;
  u8 *x;
  u8 *z;
  u8 *y;
  u8 *timer;
  u8 *step;

  flag = g_battle_work;
  flag[0x0] |= 0x20;
  stage = g_battle_work;
  stage[0x5C] = 3;
  x = g_battle_work;
  x[0x5D] = -(x[0xB] << 4);
  z = g_battle_work;
  z[0x5F] = -(z[0xB] << 4);
  y = g_battle_work;
  y[0x5E] = -(y[0xB] << 4);
  timer = g_battle_work;
  timer[0x9] = 8;
  step = g_battle_work;
  step[1]++;
  func_8014DAEC();
}
