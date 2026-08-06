#include "bof3/bof3.h"
#include "internal.h"

/* @source 0x801D67EC
 * @behavior clears two battle-state bytes and increments one global counter.
 */
void battle03_clear_state_bytes_advance(void) {
  D_8014864C = 0;
  D_801462E5 = 0;
  BATTLE_GLOBAL_BYTE_62E2 += 1;
}
