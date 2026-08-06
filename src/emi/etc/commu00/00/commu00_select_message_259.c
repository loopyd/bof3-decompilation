#include "internal.h"

/* @source 0x801F1770
 * @behavior selects message 0x259 and advances its local state byte
 */
void commu00_select_message_259(void) {
  func_80161FDC(0x259u);
  commu00_fairy_progress[0] += 1;
}
