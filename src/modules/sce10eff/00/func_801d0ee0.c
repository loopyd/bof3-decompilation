#include "bof3/defines.h"

typedef struct ScenarioSce10effScratch {
  u8  pad_00[0x08];
  u8  flags_08;
  u8  pad_09[0x2b];
  s32 unk_34;
  s32 unk_38;
  u8  pad_3c[0x02];
  s16 unk_3e;
} ScenarioSce10effScratch;

extern void func_80178b78(void);
extern void func_80179558(s16* rotation, void* matrix, void* vector);
extern void func_80179738(s16* translation, void* object_work);
extern void func_80178cb8(void* scene, void* object_work);
extern void func_80178fd8(void* object_work);
extern void func_80179068(void* object_work);

extern u8 D_801492e8[];

/* @behavior builds a temporary transform from scratchpad camera state and submits
 * it through the local scene object pipeline.
 * @source 0x801d0ee0 FUN_801d0ee0
 */
void func_801d0ee0(void) {
  ScenarioSce10effScratch* scratch;
  s16                      rotation[3];
  s16                      translation[3];
  u32                      object_work[5];
  u32                      matrix[3];
  u32                      vector[2];

  func_80178b78();

  scratch = *(ScenarioSce10effScratch* volatile*)0x1f800044;

  translation[0] = 0;
  translation[1] = 0;
  translation[2] = 0x400;
  if ((scratch->flags_08 & 1u) != 0u) {
    translation[2] = 0;
  }

  rotation[0] = (s16)(scratch->unk_34 >> 9) - 0x4000;
  rotation[1] = (s16)(scratch->unk_38 >> 9) - 0x4000;

  rotation[2] =
      (s16)((-(s32)scratch->unk_3e + ((u32)(-(s32)scratch->unk_3e) >> 31)) >>
            1);

  func_80179558(rotation, matrix, vector);
  func_80179738(translation, object_work);
  func_80178cb8(D_801492e8, object_work);
  func_80178fd8(object_work);
  func_80179068(object_work);
}
