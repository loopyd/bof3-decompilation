#include "internal.h"

/* @source 0x801E0028
 * @behavior Decrements a frame counter; when it wraps to zero, increments
 *           a secondary counter.
 */
void func_801E0028(void) {
  volatile u8* p = &D_80148654;
  u8           val = *p - 1;
  *p = val;
  if (val == 0) {
    D_80148652 += 1;
  }
}
