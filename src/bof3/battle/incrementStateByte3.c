#include "bof3/bof3.h"
/* @source 0x801E925C
 * @behavior increments byte three of one fixed battle-state object.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementStateByte3(void) {
  *(u8*)(((u8*)panelTaskRoot) + 3) += 1;
}
