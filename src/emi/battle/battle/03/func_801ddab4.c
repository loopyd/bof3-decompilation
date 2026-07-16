#include "internal.h"

/* @behavior submits one small positional effect around the current scratch object
 * when bit `0x80` is set in the input mask.
 * @source 0x801DDAB4
 */
void func_801DDAB4(u32 arg0) {
  if ((arg0 & 0x80u) != 0u) {
    func_8019651C((void*)BATTLE_LOCAL_SCRATCH_PTR, -6, -10, 0, 0);
  }
}
