#include "internal.h"

/* @source 0x801BDB7C
 * @behavior scans 3 bytes in a mode-indexed table at D_80144F5A for 0xFF;
 *            returns index of first match (0-2), or 3 if none.
 */
u8 func_801BDB7C(u8 mode) {
  s32 index = 0;
  /* MATCHING_AID: original keeps 0xFF pinned in a2 so the found-compare
     * emits `beq v0,a2`; GCC 2.7.2 cannot otherwise pin the constant to
     * that register (matching-playbook §16). */
  REGISTER_PIN(u8, sentinel, "a2") = 0xFF;
  /* MATCHING_AID: `masked` must be pinned to a0 as `unsigned int` (not u8)
     * so the `andi a0` and the `lui v1` base load schedule in the original
     * `andi` -> base-load -> `sll`/`addu` order (matching-playbook §16). */
  REGISTER_PIN(unsigned int, masked, "a0") = mode & sentinel;
  REGISTER_PIN(u8*, base, "v1") = D_80144F5A;
  s32                   off = (s32)masked * 3;
  u8*                   ptr = base + off;

  while (index < 3) {
    if (*ptr == sentinel) {
      break;
    }
    index++;
    ptr++;
  }
  return index;
}
