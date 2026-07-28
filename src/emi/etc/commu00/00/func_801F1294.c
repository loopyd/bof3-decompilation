#include "internal.h"

/* @source 0x801F1294
 * @behavior selects active UI data, requests mode 14, and refreshes it.
 */
s32 func_801F1294(u8* active_ui)
{
  COMMU00_ACTIVE_UI = active_ui;
  D_801448EB = 14;
  func_8015C088();
  return 0;
}
