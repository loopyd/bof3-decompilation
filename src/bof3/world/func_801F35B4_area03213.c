#include "bof3/world/area03213_internal.h"

/* @source 0x801F35B4
 * @behavior Submits the 0x98-byte work record of 0x80146888 selected by byte
 *           0x03 of the scratch cursor record at 0x1F800044 through
 *           func_80196718, submits the record selected by cursor byte 0x04 the
 *           same way, then runs the shared tick func_80196070.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F35B4(void) {
  func_80196718(&((World00Area032WorkRecord*)D_80146888)[D_1F800044[3]]);
  func_80196718(&((World00Area032WorkRecord*)D_80146888)[D_1F800044[4]]);
  func_80196070();
}
