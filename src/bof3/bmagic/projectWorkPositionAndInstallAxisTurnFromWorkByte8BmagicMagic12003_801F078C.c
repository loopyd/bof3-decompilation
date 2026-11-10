#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801F078C
 * @behavior Push the matrix stack and select an axis turn from work byte 8:
 *           values 0 and 2 set the second angle to 0xC00 and 0x400;
 *           values 1 and 3 set the first angle to 0x400 and 0xC00.
 *           Other values leave all three angles zero. Reload the work pointer,
 *           project words 0x34 and 0x38 shifted right nine and biased by
 *           -0x4000, and half the negated signed halfword 0x3E. Build the
 *           selected rotation, compose matrix 0x801492E8, and install both
 *           rotation and translation. Takes no arguments and returns no value;
 *           the caller restores the matrix stack after drawing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkPositionAndInstallAxisTurnFromWorkByte8BmagicMagic12003_801F078C(void) {
  u8 *work;
  s16 position[3];
  s16 rotation[3];
  MATRIX object_work;
  long flag;

  PushMatrix();
  rotation[2] = 0;
  rotation[1] = 0;
  rotation[0] = 0;
  switch (g_battle_work[8]) {
  case 0:
    rotation[1] = 0xC00;
    break;
  case 1:
    rotation[0] = 0x400;
    break;
  case 2:
    rotation[1] = 0x400;
    break;
  case 3:
    rotation[0] = 0xC00;
    break;
  }
  work = g_battle_work;
  position[0] = (s16)(*(s32 *)(work + 0x34) >> 9) - 0x4000;
  position[1] = (s16)(*(s32 *)(work + 0x38) >> 9) - 0x4000;
  position[2] = (s16)(-*(s16 *)(work + 0x3E) / 2);
  RotTrans((SVECTOR *)position, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)rotation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
