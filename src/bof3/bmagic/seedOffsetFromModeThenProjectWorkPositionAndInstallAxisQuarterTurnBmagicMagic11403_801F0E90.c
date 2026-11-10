#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */
extern void func_801DD350(s32 work); /* @source 0x801DD350 */

/* @source 0x801F0E90
 * @behavior Push the matrix stack and select a quarter turn around the first
 *           axis when work byte 8 is odd, or the second axis when it is even.
 *           Seed work words C and 10 with 0xC000 and zero, then transform that
 *           pair through helper 0x801DD350. Add the pair to shared record
 *           0x801EB4E0 words 34 and 38 and copy its halfword 3E plus 0x200.
 *           Project the resulting work position with RotTrans, build the
 *           selected rotation with RotMatrix, compose matrix 0x801492E8 and
 *           install both rotation and translation. Takes no arguments and
 *           returns no value; the caller later restores the matrix stack.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedOffsetFromModeThenProjectWorkPositionAndInstallAxisQuarterTurnBmagicMagic11403_801F0E90(void) {
  u8 *offset_work;
  u8 *record;
  u8 *work;
  s16 position[3];
  s16 rotation[3];
  MATRIX object_work;
  long flag;

  PushMatrix();
  if (g_battle_work[8] & 1) {
    rotation[0] = 0x400;
    rotation[1] = 0;
  } else {
    rotation[0] = 0;
    rotation[1] = 0x400;
  }
  rotation[2] = 0;
  offset_work = g_battle_work;
  *(s32 *)(offset_work + 0xC) = 0xC000;
  *(s32 *)(offset_work + 0x10) = 0;
  func_801DD350((s32)offset_work);
  record = D_801EB4E0;
  work = g_battle_work;
  *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34) + *(s32 *)(work + 0xC);
  *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38) + *(s32 *)(work + 0x10);
  *(u16 *)(work + 0x3E) = *(u16 *)(record + 0x3E) + 0x200;
  position[0] = (s16)(*(s32 *)(work + 0x34) >> 9) - 0x4000;
  position[1] = (s16)(*(s32 *)(work + 0x38) >> 9) - 0x4000;
  position[2] = (s16)(-*(s16 *)(work + 0x3E) / 2);
  RotTrans((SVECTOR *)position, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)rotation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
