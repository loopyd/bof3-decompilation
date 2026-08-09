#include "bof3/battle/battle03_internal.h"

/* @source 0x801E94D0
 * @behavior copies 0x801D0FC4's three handler words to its stack, then calls
 * the entry selected by byte +0x03 of the battle-state object. Original table
 * bytes are 0x801E9530, 0x801E9538, and 0x801EAB64; each is void(void).
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchStateByte3TableFc4(void) {
  Battle03DispatchTable handlers;

  handlers = D_801D0FC4;
  handlers.handlers[((u8*)D_80148648)[3]]();
}
