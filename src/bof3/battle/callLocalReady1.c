#include "bof3/battle/battle03_internal.h"

/* @source 0x801E1AE4
 * @behavior Calls the preceding battle handler.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void callLocalReady1(void) {
    localReadyOrHelper1();
}
