#include "bof3/context.h"
#include "bof3/defines.h"
#include "internal.h"

extern u_long D_801D8BB0;

/* @behavior probes the independently loaded LOGO.EXE stream path for the authored
 * SLUS boot harness.
 * @source Not native SLUS code: 0x801ce760 and 0x801cea98 belong to LOGO.EXE.
 */
const SlotTableEntry* slot_table_logo_str(void) {
  volatile u8 scratch[0x18];
  u_long      pad_state;
  int         stream_finished;

  (void)scratch;
  func_801ce758();
  CdInit();
  func_801ce760((void*)0x8003b800, D_801D8BB0);
  do {
    pad_state = PadRead(0);
    if ((pad_state & 0x800) != 0) {
      break;
    }
    stream_finished = func_801cea98();
  } while (stream_finished == 0);
  func_801cebfc();
  StopCallback();
  PadStop();
  SetDispMask(0);
}
