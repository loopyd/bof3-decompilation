#include "internal.h"

/* @behavior initializes one tiny ui state bundle with a fixed mode byte and one
 * caller-provided byte.
 * @source 0x801d93e4 FUN_801d93e4
 */
void func_801d93e4(u8 arg0) {
  func_80158db8(3u, 3u);
  *(u8*)0x8014839eu = 2u;
  *(u8*)0x8014839fu = arg0;
}
