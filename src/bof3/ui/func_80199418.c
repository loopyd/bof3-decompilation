#include "bof3/ui/game00_internal.h"

/**
 * @source 0x80199418
 * @behavior runs the func_801D0D80 update slice, then finalizes the shared
 * front-end frame through func_80158C80.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_80199418(void) {
  func_801D0D80();
  func_80158C80();
}
