#include "bof3/bof3.h"

extern u32 D_80148648;

/* @source 0x801E925C
 * @behavior increments byte three of one fixed battle-state object.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void incrementStateByte3(void) {
  *(u8*)(D_80148648 + 3) += 1;
}
