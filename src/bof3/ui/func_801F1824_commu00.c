#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1824
 * @behavior Sets the 0x20 transfer-ready flag in the shared EMI loader state
 * byte, requests the stream slot for the low seven bits of the stream index
 * hint in family 0, and advances the fairy progress byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F1824(void) {
  D_80146854 = 0x20;
  func_8016728C(D_80145024 & 0x7F, 0);
  D_801448EC += 1;
}
