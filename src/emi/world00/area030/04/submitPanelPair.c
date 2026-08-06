#include "internal.h"

/* @behavior submits the two AREA030 panel updates at the requested vertical
 * positions after selecting graphics mode 0.
 * @source 0x801E0ED0
 */
void submitPanelPair(s16 arg0, s16 arg1) {
  submitTpageDrawMode(0, 1);
  func_801E0DCC(0, 1, (s16)(arg0 + 8), arg1);
  func_801E0DCC(1, 1, (s16)(arg0 + 0x108), arg1);
}
