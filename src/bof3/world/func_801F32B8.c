#include "bof3/world/area02613_internal.h"

/* @source 0x801F32B8
 * @behavior publishes the D_80145E90 work base as the scratchpad work cursor and stores the helper result byte at cursor+0xB unless it reports 0xFF.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 32/32 instructions, 128 bytes, live byte match. The helper is
 * called with (1, 0, D_801F4CE0[D_80144F5A], D_80145EBE, D_80145EC0); the
 * cursor is re-read from 0x1F800044 after the call so the byte lands in the
 * published helper-visible record.
 */
void func_801F32B8(void) {
  u8 result;

  D_1F800044 = D_80145E90;
  result = func_8015CB18(1, 0, D_801F4CE0[D_80144F5A], D_80145EBE, D_80145EC0);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
