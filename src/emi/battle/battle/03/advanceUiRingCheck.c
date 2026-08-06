#include "internal.h"

/* @behavior advances the UI ring index modulo 16 and reports whether it reached the
 * current target index.
 * @source 0x801EAB38
 */
u8 advanceUiRingCheck(void) {
  u8* const base = (u8*)0x801f0000u;
  s16       target_offset;
  s16       index_offset;
  u8        index;
  u8        target;

  target_offset = (s16)0xc328u;
  index_offset = (s16)0xbf04u;
  index = base[index_offset];
  target = base[target_offset];
  index = (index + 1u) & 0x0fu;
  base[index_offset] = index;
  return (target ^ index) == 0u;
}
