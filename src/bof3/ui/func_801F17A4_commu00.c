#include "bof3/ui/commu00_internal.h"

/* @source 0x801F17A4
 * @behavior Once the EMI loader reports the streamed slot ready, calls the
 * shared 0x8014E284 routine, publishes 1 into D_80145988, clears the four bytes
 * at D_801F294C and the two bytes at D_801F2944, and advances the fairy
 * progress byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F17A4(void) {
  if (func_80162D00() == 0) {
    return;
  }
  func_8014E284();
  D_80145988 = 1;
  D_801F294C[0] = 0;
  D_801F294C[1] = 0;
  D_801F294C[2] = 0;
  D_801F294C[3] = 0;
  D_801F2944[0] = 0;
  D_801F2944[1] = 0;
  fairyProgress[0] += 1;
}
