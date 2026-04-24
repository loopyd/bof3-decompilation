#include "internal.h"
#include "bof3/original_symbols.h"

void func_801ceef4(void);

/* possible name: logo_exe_main
 * does: enters LOGO.EXE, boots the CAPCOM30.STR stream, polls for skip or
 * completion, then shuts the video branch down.
 * @source: 0x801cedfc FUN_801cedfc
 */
void func_801cedfc(void) {
  volatile u8 scratch[0x18];
  u_long      pad_state;
  int         stream_finished;

  (void)scratch;
  func_801ce758();
  CdInit();
  func_801ce760((void*)0x8003b800, *(u_long*)0x801d8bb0);
  do {
    pad_state = PadRead(0);
    if ((pad_state & 0x800) != 0) {
      break;
    }
    stream_finished = func_801cea98();
  } while (stream_finished == 0);
  func_801cebfc();
  StopCallback();
  func_801ceef4();
  SetDispMask(0);
}
