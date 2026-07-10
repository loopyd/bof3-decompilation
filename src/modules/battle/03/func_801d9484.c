#include "internal.h"

/* does: initializes one small ui state bundle with fixed bytes and halfwords.
 * @source: 0x801d9484 FUN_801d9484
 */
void func_801d9484(void) {
  func_80158db8(0u, 3u);
  *(u8*)0x80148332u = 6;
  *(u16*)0x80148334u = 0x14;
  *(u8*)0x80148333u = 0;
  *(s16*)0x80148336u = -0x16;
}
