#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1294
 * @behavior selects active UI data, requests mode 14, and refreshes it.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 selectUiMode14(u8* active_ui)
{
  COMMU00_ACTIVE_UI = active_ui;
  uiMode = 14;
  func_8015C088();
  return 0;
}
