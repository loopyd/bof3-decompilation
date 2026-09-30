#include "bof3/world/area02414_internal.h"

/* @behavior overlay step: passes scratch-work byte 0x09 to func_801F3E48, then
 * advances that byte by two; once it reaches 0x80 it calls the shared cue
 * dispatcher 0x8015DF18 with 0x206, 0x20E and 0x203, restarts scratch-work
 * byte 0x09 at zero and advances scratch-work byte 0x01.
 * @source 0x801F2CB0
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F2CB0(void) {
  func_801F3E48(D_1F800044[9]);
  D_1F800044[9] += 2;

  if (D_1F800044[9] >= 0x80u) {
    func_8015DF18(0x206);
    func_8015DF18(0x20E);
    func_8015DF18(0x203);
    D_1F800044[9] = 0;
    D_1F800044[1]++;
  }
}
