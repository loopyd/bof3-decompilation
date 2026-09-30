#include "bof3/battle/battle15_internal.h"

/* @source 0x800B258C
 * @behavior Reads the signed panel x and field_06 halfwords plus the byte at
 * +0xB, then forwards them to func_800B4220.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800B258C(void) {
  u8* panel = (u8*)g_PanelTaskRoot;

  func_800B4220(FIELD_REF(s16, panel, 4), FIELD_REF(s16, panel, 6),
                FIELD_REF(u8, panel, 0xB));
}
