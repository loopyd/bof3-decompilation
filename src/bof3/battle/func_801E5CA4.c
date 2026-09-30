#include "bof3/battle/battle03_internal.h"

/* @behavior Copies the five queued-slot handlers off 0x801D0EF4 to the stack and
 * dispatches the one selected by the current queued slot byte +0x01; when
 * scratch work flag bit 0 is set it then dispatches again on queued slot byte
 * +0x07: state 0 forwards slot byte +0x27 to func_801E62BC unless slot byte
 * +0x01 has reached 3, in which case it forwards the recomputed halfword +0x36,
 * the signed halfword +0x3A, byte +0x27 and halfword +0x60 to func_801D9684,
 * while states 1..3 draw the icon strip at the slot halfwords +0x34/+0x38 with
 * delay 2, 1 and 0.
 * @source 0x801E5CA4
 * @status exact
 * @match 100.00
 * @residual none
 */
void NO_SIBLING_CALLS func_801E5CA4(void) {
  Battle03FiveDispatchTable table;
  Battle03QueuedSlot*       slot;

  table = D_801D0EF4;
  table.handlers[FIELD_REF(u8, D_801EC2E0, 0x01u)]();

  if ((D_1F800044->flags_00 & 1u) == 0u) {
    return;
  }

  slot = D_801EC2E0;
  switch (FIELD_REF(u8, slot, 0x07u)) {
  case 0:
    if (FIELD_REF(u8, slot, 0x01u) < 3u) {
      func_801E62BC(FIELD_REF(u8, slot, 0x27u));
    } else {
      func_801D9684((s16)(FIELD_REF(u16, slot, 0x36u) - 12),
                    FIELD_REF(s16, slot, 0x3au), FIELD_REF(u8, slot, 0x27u),
                    FIELD_REF(u16, slot, 0x60u));
    }
    break;
  case 1:
    drawIconStrip24x8(FIELD_REF(u16, slot, 0x36u), FIELD_REF(s16, slot, 0x3au),
                      FIELD_REF(u8, slot, 0x27u), 2);
    break;
  case 2:
    drawIconStrip24x8(FIELD_REF(u16, slot, 0x36u), FIELD_REF(s16, slot, 0x3au),
                      FIELD_REF(u8, slot, 0x27u), 1);
    break;
  case 3:
    drawIconStrip24x8(FIELD_REF(u16, slot, 0x36u), FIELD_REF(s16, slot, 0x3au),
                      FIELD_REF(u8, slot, 0x27u), 0);
    break;
  case 4:
    break;
  }
}
