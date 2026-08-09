#include "bof3/core/slus_internal.h"

/* @behavior stores one frontend-local mode and installs the matching callback.
 * @source 0x8014ECAC
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void __attribute__((noinline)) setFrontLocalMode(u16 local_mode) {
  volatile u16* gameFront;

  gameFront = (volatile u16*)0x80140000u;

  if (gameFront[0x3c40 / sizeof(u16)] != 0u) {
    return;
  }

  gameFront[0x3c90 / sizeof(u16)] = local_mode & 0xffu;
  installCallbackSlot(2, frontLocalModeCallbackLoop);
}
