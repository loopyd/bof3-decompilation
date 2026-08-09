#include "bof3/battle/battle03_internal.h"

/* @behavior initializes one small ui state bundle with fixed bytes and halfwords.
 * @source 0x801D9484
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void initUiBundleSlot0(void) {
  func_80158DB8(0u, 3u);
  *(u8*)0x80148332u = 6;
  *(u16*)0x80148334u = 0x14;
  *(u8*)0x80148333u = 0;
  *(s16*)0x80148336u = -0x16;
}
