#include "bof3/world/area00813_internal.h"

extern void func_8014D6B8(u32 flag);
extern void func_8015DF18(u16 cue);

/* @source 0x801F3010
 * @behavior clears scratch byte 9, selects area mode 9 when the shared
 *           high-bit flag is set; otherwise installs the local area state as
 *           both the current state and the scratchpad state, sets bit 0x40 in
 *           the shared state byte 0x801460E8, dispatches the shared cues 0x20E
 *           and 0x57, restores the previous scratchpad state pointer and
 *           selects mode 7 on it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3010(void) {
  volatile World00Area008State* previous;

  g_areaWork->unk_09 = 0;
  if ((D_80146867 & 0x80u) != 0u) {
    g_areaWork->mode = 9;
    return;
  }

  previous = g_areaWork;
  currentState = &areaState;
  g_areaWork = &areaState;
  D_801460E8 |= 0x40;
  func_8015DF18(0x20E);
  func_8014D6B8(0x57);
  g_areaWork = previous;
  previous->mode = 7;
}
