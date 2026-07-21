#include "internal.h"

/* @source 0x801E262C
 * @behavior sets D_80148650 to 1, and D_80148651/D_80148652 to 0.
 */
void func_801E262C(void) {
  D_80148650 = 1;
  D_80148651 = 0;
  D_80148652 = 0;
}
