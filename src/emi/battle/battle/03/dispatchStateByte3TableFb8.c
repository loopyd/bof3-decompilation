#include "internal.h"

/* @source 0x801E943C
 * @behavior copies the locally owned three-word dispatch table at 0x801D0FB8,
 * then calls the void(void) entry selected by byte +0x03 of the battle-state
 * object. Original table words are 0x801EAB64, 0x801E949C, and 0x801EAB64.
 * `callD8450WhenIdle` is a target-local void(void) boundary: it supplies its own
 * `a0` before its direct call and does not consume caller-provided arguments.
 */
void dispatchStateByte3TableFb8(void) {
  Battle03DispatchTable handlers;

  handlers = D_801D0FB8;
  handlers.handlers[((u8*)D_80148648)[3]]();
}
