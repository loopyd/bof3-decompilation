#include "internal.h"

/* possible name: logo_exe_main
 * @behavior enters LOGO.EXE, boots the CAPCOM30.STR stream, polls for skip or
 * completion, then shuts the video branch down.
 * @source 0x801CEDFC
 */
void func_801CEDFC(void) {
  volatile u8 scratch[0x18];
  u_long      pad_state;
  int         stream_finished;

  (void)scratch;
  func_801CE758();
  CdInit();
  func_801CE760((void*)0x8003b800, D_801D8BB0);
  do {
    pad_state = PadRead(0);
    if ((pad_state & PADstart) != 0) {
      break;
    }
    stream_finished = func_801CEA98();
  } while (stream_finished == 0);
  func_801CEBFC();
  StopCallback();
  func_801CEEF4();
  SetDispMask(0);
}
