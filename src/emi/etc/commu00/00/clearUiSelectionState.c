#include "internal.h"

/* @source 0x801F1684
 * @behavior clears the three frontend UI selection-state bytes.
 */
void clearUiSelectionState(void)
{
  func_8015C058();
  uiMode = 0;
  fairyProgress[0] = 0;
  fairySlotIndex = 0;
}
