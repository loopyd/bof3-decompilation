#include "internal.h"

extern u8 D_801492E8[];

/* @behavior builds a temporary transform from scratchpad camera state and submits
 * it through the local scene object pipeline.
 * @source 0x801D0EE0
 */
void func_801D0EE0(void) {
  ScenarioSce10effScratch* scratch;
  s16                      rotation[3];
  s16                      translation[3];
  MATRIX                     object_work;
  long                       flag;

  PushMatrix();

  scratch = SPAD_PTR_SLOT(ScenarioSce10effScratch, 0x44u);

  translation[0] = 0;
  translation[1] = 0;
  translation[2] = 0x400;
  if ((scratch->flags_08 & 1u) != 0u) {
    translation[2] = 0;
  }

  rotation[0] = (s16)(scratch->unk_34 >> 9) - 0x4000;
  rotation[1] = (s16)(scratch->unk_38 >> 9) - 0x4000;

  rotation[2] = (s16)(-(s32)scratch->unk_3e / 2);

  RotTrans((SVECTOR*)rotation, (VECTOR*)object_work.t, &flag);
  RotMatrix((SVECTOR*)translation, (MATRIX*)&object_work);
  MulMatrix2((MATRIX*)D_801492E8, (MATRIX*)&object_work);
  SetRotMatrix((MATRIX*)&object_work);
  SetTransMatrix((MATRIX*)&object_work);
}
