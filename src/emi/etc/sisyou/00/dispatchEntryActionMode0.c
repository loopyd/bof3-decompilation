#include "internal.h"

/* @source 0x801D2C64
 * @behavior dispatches the selected entry's +0x11 action.
 */
void dispatchEntryActionMode0(void) {
  func_801D10AC((u16)(masterActionBaseTable[masterIndex] + 0x11));
}
