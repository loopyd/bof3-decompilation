#include "bof3/ui/game00_internal.h"

extern u8 func_801B4D7C(u8 arg0);
extern u8 func_801B44DC(u8 arg0);

/*
 * @source 0x801B4494
 * @behavior Selects one of two sibling work routines by the work-area speed
 * byte at 0x70 (speed_70) reached through the scratchpad pointer slot
 * 0x1F800044: a non-zero speed byte calls func_801B4D7C and a zero speed byte
 * calls func_801B44DC, and the byte status returned by the selected routine is
 * returned. Both call sites mask the request id to eight bits, which is the
 * argument width the two routines take.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 dispatchWorkHandlerBySpeedByte_game00_801B4494(s32 id) {
  struct GameWorkArea* work = SPAD_PTR_SLOT(struct GameWorkArea, 0x44u);
  u8 result;

  if (work->speed_70 != 0) {
    result = func_801B4D7C(id);
  } else {
    result = func_801B44DC(id);
  }
  return result;
}
