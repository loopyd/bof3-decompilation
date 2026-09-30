#include "bof3/battle/battle03_internal.h"

/* @behavior Initializes the mode-1 ui state bundle: requests ui bundle mode 1
 * from the shared allocator, publishes the zeroed frame bytes, the caller byte
 * and the current global mode byte at 0x801462F0, then stores the 0xF0 halfword
 * and the per-mode byte read from the 0x801EAF27 table.
 * @source 0x801D9304
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D9304(u8 arg0) {
  u8  mode;
  u16 value;

  func_80158DB8(1u, 3u);
  *(u8*)0x80148356u = 0;
  *(u8*)0x80148357u = arg0;
  mode = D_801462F0;
  *(u8*)0x8014835Eu = mode;
  value = D_801EAF27[mode];
  *(u16*)0x8014835Au = 0xF0;
  *(u8*)0x8014835Cu = 0;
  *(u8*)0x8014835Du = 0;
  *(u16*)0x80148358u = value;
}
