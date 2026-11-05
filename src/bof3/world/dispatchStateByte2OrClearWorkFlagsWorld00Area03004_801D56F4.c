#include "bof3/world/area03004_internal.h"

extern u8 gateByteWorld00Area03004_80144288; /* @source 0x80144288 @kind unknown */
extern void (*stateByte2HandlersWorld00Area03004_801E1FBC[])(void); /* @source 0x801E1FBC @kind data */

/* @source 0x801D56F4
 * @behavior Clears the work-record header through func_80196070 when the
 * shared gate byte at 0x80144288 is nonzero or byte 0x80144281 is not 14.
 * Otherwise dispatches work-record byte 2 through the table at 0x801E1FBC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2OrClearWorkFlagsWorld00Area03004_801D56F4(void) {
  if (gateByteWorld00Area03004_80144288 != 0 || D_80144281 != 14) {
    func_80196070();
  } else {
    stateByte2HandlersWorld00Area03004_801E1FBC[SPAD_PTR_SLOT(u8, 0x44)[2]]();
  }
}
