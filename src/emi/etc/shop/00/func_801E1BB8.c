#include "internal.h"

/* @source 0x801E1BB8
 * @behavior swaps two u8 values pointed to by arg0 and arg1.
 */
void func_801E1BB8(u8* arg0, u8* arg1) {
  u8 temp;

  temp = *arg0;
  *arg0 = *arg1;
  *arg1 = temp;
}
