#include "bof3/ui/game00_internal.h"

/* @behavior Searches the 28 world-handler entries at worldIdHandlerTable (byte
 * key at +0, handler pointer at +4, 8-byte stride) for the entry whose key
 * equals the pending world id D_80143F00 and calls the matching handler with
 * the low halfwords of the two arguments, returning its signed byte result;
 * returns 0 when no entry matches.
 * @source 0x801A8CB4
 * @status exact
 * @match 100.00
 * @residual none
 */
s8 dispatchWorldIdHandler(s32 arg0, s32 arg1) {
  u16 key;
  s32 i;

  key = D_80143F00;
  for (i = 0; i < 28; i++) {
    if (worldIdHandlerTable[i].key_00 == key) {
      break;
    }
  }
  if (i == 28) {
    return 0;
  }
  return worldIdHandlerTable[i].handler_04((u16)arg0, (u16)arg1);
}
