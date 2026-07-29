#include "internal.h"

/* @source 0x801E91FC
 * @behavior copies the locally owned three-word dispatch table at 0x801D0FAC,
 * then calls the void(void) entry selected by battle-state byte +0x03.
 * Original table words are 0x801E927C, 0x801E9390, and 0x801EAB64.
 */
void func_801E91FC(void) {
  Battle03DispatchTable handlers;

  handlers = D_801D0FAC;
  handlers.handlers[((u8*)D_80148648)[3]]();
}
