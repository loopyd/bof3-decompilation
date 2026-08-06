#include "internal.h"

extern void func_801F3B00(s32 arg0, s32 arg1);

/* @behavior advances local scratch state `0x02` when the shared byte reaches `2`,
 * then calls the `0x801f3b00` local step with the scratch halfword at `0x2e`.
 * @source 0x801F35B8
 */
void advanceState02Step(void) {
  const u8* global;
  u32       state;

  global = (const u8*)0x80140000u;
  state = global[0x54f2];
  if (state == 2) {
    WORLD00_AREA016_SCRATCH_PTR->state_02 = 3;
  }

  {
    World00Area016Scratch* scratch;

    scratch = WORLD00_AREA016_SCRATCH_PTR;
    func_801F3B00(0x10, scratch->field_2e);
  }
}
