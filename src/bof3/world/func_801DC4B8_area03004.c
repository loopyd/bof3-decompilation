#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 level-flag retirement: calls the shared frontend slot setter
 * func_801665A0 with (3, the byte of the shared AREA030 level flag 0x80145026,
 * 1, 0) and, when that helper reports a zero low byte, clears the level flag
 * 0x80145026 together with its companion byte 0x80145027; the target-local
 * overlay step confirmSelectionStep then runs unconditionally.
 * @source 0x801DC4B8
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DC4B8(void) {
  u8* flag = &D_80145026;

  if (func_801665A0(3, *flag, 1, 0) == 0) {
    *flag = 0;
    D_80145027 = 0;
  }
  confirmSelectionStep();
}
