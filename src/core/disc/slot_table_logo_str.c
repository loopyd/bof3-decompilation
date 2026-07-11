#include "bof3/context.h"
#include "bof3/defines.h"
#include "internal.h"

extern u_long DAT_801d8bb0;

void SetDispMask(int mask);

void func_801ce758(void);
void func_801ceef4(void);

/* @behavior boots the LOGO.EXE stream path, polls for skip/completion, then shuts
 * the display branch down.
 * Not authoritative for LOGO.EXE source mapping; see
 * src/modules/logo/func_801cedfc.c for 0x801cedfc.
 */
const SlotTableEntry* slot_table_logo_str(void) {
  volatile u8 scratch[0x18];
  u_long      pad_state;
  int         stream_finished;

  (void)scratch;
  func_801ce758();
  CdInit();
  func_801ce760((void*)0x8003b800, DAT_801d8bb0);
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
