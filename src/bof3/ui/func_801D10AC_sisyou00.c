#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D10AC
 * @behavior reads the shared pad-state word at 0x80145AA8 and, when it shares
 *           a bit with the 0x80145AC2 pressed-button mask, steps the handler
 *           index D_801D4286 down when the D_801D4284 flag is set and
 *           otherwise queues frontend cue 0x102 and steps it up, then queues
 *           cue 0x104; when the pad word instead shares a bit with the
 *           0x80145AC4 mask it sets D_801D4284, steps the handler index down
 *           and queues cue 0x106; when neither mask matches and the direction
 *           mask returned by func_8014E640 has a set low halfword bit, it
 *           queues cue 0x100 and toggles D_801D4284. It then draws every one
 *           of the D_80146254 0x140-byte-stride records at 0x80145FCC through
 *           func_801D177C and func_801D1444, emits the panel rectangle
 *           through func_801D1CC0 and the shared panel strip through
 *           func_801D1E54, draws the texture whose handle is the main-RAM
 *           base 0x80010000 plus the D_80010000 offset-table entry selected
 *           by the argument through func_8014F800, and finally submits the
 *           panel task at (D_801D4284 * 32 + 0xF3, 0x15) through
 *           func_801647C4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D10AC(u32 arg0) {
  s32 direction;
  s32 i;

  direction = func_8014E640(D_80145AA8[0] & 0xA000);

  if (D_80145AA8[0] & D_80145AC2) {
    if (D_801D4284) {
      D_801D4286 = D_801D4286 - 1;
    } else {
      game_queue_frontend_cue(0x102);
      D_801D4286 = D_801D4286 + 1;
    }
    game_queue_frontend_cue(0x104);
  } else if (D_80145AA8[0] & D_80145AC4) {
    D_801D4284 = 1;
    D_801D4286 = D_801D4286 - 1;
    game_queue_frontend_cue(0x106);
  } else if ((direction & 0xFFFF) != 0) {
    game_queue_frontend_cue(0x100);
    D_801D4284 = D_801D4284 ^ 1;
  }

  for (i = 0; i < D_80146254; i++) {
    func_801D177C(0x11, 0x3E + i * 0x36, D_80145FCC[i * 0x140], i);
    func_801D1444(0x89, 0x3E + i * 0x36, D_80145FCC[i * 0x140]);
  }

  func_801D1CC0(0x11, (D_801D428A * 0x36 + 0x3E) & 0xFFFE, 0x110, 0x34, 0, 6);
  func_801D1E54(0x14, 0x10, 0x118, 0x13, D_80144952);
  func_8014F800(0x1B, 0x13, 0, 0xFF,
                0x80010000u + ((u16*)0x80010000)[arg0 & 0xFFFF]);
  func_801647C4(D_801D4284 * 32 + 0xF3, 0x15, 0);
}
