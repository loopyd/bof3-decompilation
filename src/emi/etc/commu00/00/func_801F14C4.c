#include "internal.h"

/* @source 0x801F14C4
 * @behavior selects active UI data, requests mode 23, and refreshes it.
 */
s32 func_801F14C4(u8* active_ui)
{
  COMMU00_ACTIVE_UI = active_ui;
  D_801448EB = 23;
  func_8015C088();
  return 0;
}
