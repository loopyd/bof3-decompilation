#include "internal.h"

/* @source 0x801D3350
 * @behavior dispatches the selected entry's +0x11 action.
 */
void func_801D3350(void) {
  func_801D10AC((u16)(D_801D41BC[D_801448ED] + 0x11));
}
