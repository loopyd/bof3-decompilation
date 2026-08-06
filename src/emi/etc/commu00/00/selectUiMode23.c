#include "internal.h"

/* @source 0x801F14C4
 * @behavior selects active UI data, requests mode 23, and refreshes it.
 */
s32 selectUiMode23(u8* active_ui)
{
  COMMU00_ACTIVE_UI = active_ui;
  uiMode = 23;
  func_8015C088();
  return 0;
}
