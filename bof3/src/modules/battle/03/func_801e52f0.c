#include "internal.h"

/* does: submits one effect id unless the caller passes `-1`.
 * @source: 0x801e52f0 FUN_801e52f0
 */
void func_801e52f0(s16 arg0) {
  u16 effect_id;

  effect_id = (u16)arg0;
  if (effect_id != 0xffffu) {
    func_8015df18(effect_id);
  }
}
