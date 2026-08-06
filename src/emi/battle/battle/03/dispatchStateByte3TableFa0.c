#include "internal.h"

/* @behavior dispatches one of three locally owned battle handlers by state byte +0x03.
 * Table words at 0x801D0FA0 are 0x801E915C, 0x801E91CC, and 0x801EAB64.
 * @source 0x801E90FC
 */
void dispatchStateByte3TableFa0(void) {
  Battle03DispatchTable handlers;

  handlers = D_801D0FA0;
  handlers.handlers[((u8*)D_80148648)[3]]();
}
