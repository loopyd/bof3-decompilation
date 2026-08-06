#include "internal.h"

/* @source 0x801D3350
 * @behavior dispatches the selected entry's +0x11 action.
 */
void dispatchEntryActionMode1(void) {
  func_801D10AC((u16)(masterActionBaseTable[masterIndex] + 0x11));
}
