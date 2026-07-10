#include "internal.h"

/* does: advances the local state/velocity table across 27 slots, then feeds
 * each updated slot through the shared transform helper.
 * @source: 0x801f362c FUN_801f362c
 */
void func_801f362c(void) {
  volatile s16*      state_base;
  volatile s16*      state_offset;
  volatile u8*       vertex_dst;
  const volatile u8* vertex_src;
  u8                 i;

  state_base = WORLD00_AREA024_STATE_BASE;
  state_offset = WORLD00_AREA024_STATE_OFFSET;
  vertex_dst = WORLD00_AREA024_VERTEX_DST;
  vertex_src = WORLD00_AREA024_VERTEX_SRC;
  i = 0u;

  do {
    state_offset[4] =
        (s16)(state_offset[4] + (s16)(WORLD00_AREA024_GLOBAL_HALF_3E6C & 1u));
    state_base[0] = (s16)(state_base[0] + state_offset[2]);
    state_offset[-1] = (s16)(state_offset[-1] + state_offset[3]);
    state_offset[0] = (s16)(state_offset[0] + state_offset[4]);

    func_801f3708((void*)vertex_dst, (const void*)vertex_src, (s16*)state_base);

    state_offset += 0x0c;
    state_base += 0x0c;
    vertex_dst += 0x28;
    vertex_src += 0x28;
    i += 1u;
  } while (i < 0x1bu);
}
