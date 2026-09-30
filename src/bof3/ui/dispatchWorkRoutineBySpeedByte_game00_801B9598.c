#include "bof3/ui/game00_internal.h"

extern u8 func_801B95E0(void);
extern u8 func_801BA678(void);

/*
 * @source 0x801B9598
 * @behavior Selects one of two sibling work routines by the work-area speed
 * byte at 0x70 (speed_70) reached through the scratchpad pointer slot
 * 0x1F800044: a zero speed byte calls func_801B95E0 and a non-zero speed byte
 * calls func_801BA678, and the byte status returned by the selected routine is
 * returned. Both callees take no argument, unlike the request-id pair selected
 * by dispatchWorkHandlerBySpeedByte_game00_801B4494.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 dispatchWorkRoutineBySpeedByte_game00_801B9598(void) {
  struct GameWorkArea* work = SPAD_PTR_SLOT(struct GameWorkArea, 0x44u);
  u8 result;

  if (work->speed_70 == 0) {
    result = func_801B95E0();
  } else {
    result = func_801BA678();
  }
  return result;
}
