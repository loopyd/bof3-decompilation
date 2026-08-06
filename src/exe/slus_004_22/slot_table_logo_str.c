#include "bof3/context.h"
#include "base/types.h"
#include "internal.h"

extern u_long capcomStrLba;

/* @behavior probes the independently loaded LOGO.EXE stream path for the authored
 * SLUS boot harness.
 * @source Not native SLUS code: 0x801ce760 and 0x801cea98 belong to LOGO.EXE.
 */
const SlotTableEntry* slot_table_logo_str(void) {
  volatile u8 scratch[0x18];
  u_long      pad_state;
  int         stream_finished;

  (void)scratch;
  func_801CE758();
  CdInit();
  initWorkAreaAndStartSubsystems((void*)0x8003b800, capcomStrLba);
  do {
    pad_state = PadRead(0);
    if ((pad_state & PADstart) != 0) {
      break;
    }
    stream_finished = func_801CEA98();
  } while (stream_finished == 0);
  func_801CEBFC();
  StopCallback();
  PadStop();
  SetDispMask(0);
}
