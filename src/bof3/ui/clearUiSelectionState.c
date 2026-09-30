#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1684
 * @behavior clears the three frontend UI selection-state bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearUiSelectionState(void)
{
  func_8015C058();
  uiMode = 0;
  fairyProgress[0] = 0;
  fairySlotIndex = 0;
}
