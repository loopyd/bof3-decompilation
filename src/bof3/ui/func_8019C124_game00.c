#include "bof3/ui/game00_internal.h"

/**
 * @source 0x8019C124
 * @behavior Snaps the active work-area coordinate whose low halfword at 0x34 is
 * nonzero (falling back to the word at 0x38 when it is zero) onto its integer
 * part and stores the result at 0x18: with a positive delta at 0x0C (or 0x10)
 * the low byte of arg0 truncates, otherwise the coordinate is rounded up by
 * 0x10000; a negative delta truncates and a zero delta stores nothing. Either
 * axis then sets byte 0x01 of the work area to 3.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8019C124(s32 arg0) {
  struct GameWorkArea* work;

  work = g_game_work;
  if (*(u16*)&work->coord_x_34 != 0) {
    if (work->field_0C > 0) {
      if ((arg0 & 0xFF) != 0) {
        work->unk_18 = work->coord_x_34 & 0xFFFF0000;
      } else {
        work->unk_18 = (work->coord_x_34 + 0x10000) & 0xFFFF0000;
      }
    } else if (work->field_0C < 0) {
      work->unk_18 = work->coord_x_34 & 0xFFFF0000;
    }
    g_game_work->unk_01 = 3;
  } else if (*(u16*)&work->coord_y_38 != 0) {
    if (work->field_10 > 0) {
      if ((arg0 & 0xFF) != 0) {
        work->unk_18 = work->coord_y_38 & 0xFFFF0000;
      } else {
        work->unk_18 = (work->coord_y_38 + 0x10000) & 0xFFFF0000;
      }
    } else if (work->field_10 < 0) {
      work->unk_18 = work->coord_y_38 & 0xFFFF0000;
    }
    g_game_work->unk_01 = 3;
  }
}
