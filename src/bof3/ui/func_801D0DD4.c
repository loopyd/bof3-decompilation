#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D0DD4
 * @behavior handles the pad selector of the master-list entry action: when the
 *           shared pad-state word at 0x80145AA8 shares a bit with the
 *           0x80145AC2 mask it compares the owner byte of the 164-byte record
 *           selected by the D_801D428A step against masterIndex: the cue-0x107
 *           path runs when the record already belongs to masterIndex and the
 *           flag is clear or when it does not and the flag is set, and the
 *           other path queues cue 0x104, clears D_801D4284 and
 *           advances the handler index; when the word instead shares a bit
 *           with the 0x80145AC4 mask it sets D_801D4284, queues cues 0x102 and
 *           0x106 and stores handler index 4; when neither mask matches, the
 *           func_8014E640 direction result's 0x1000 bit steps the D_801D428A
 *           step byte down (wrapping to the D_80146254 limit) and its 0x4000
 *           bit steps it up (wrapping to 0), each queueing cue 0x100. It then
 *           draws every one of the D_80146254 0x140-byte-stride records at
 *           0x80145FCC through func_801D177C and func_801D1444, emits the panel
 *           rectangle through func_801D1CC0 and the shared panel strip through
 *           func_801D1E54, and draws the texture whose handle is the main-RAM
 *           base 0x80010000 plus the D_80010000 offset-table entry selected by
 *           the argument through func_8014F800.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D0DD4(u16 action, s32 flag) {
  s32 direction;
  s32 index;
  s32 offset;
  u8 value;

  value = flag;
  direction = func_8014E640(D_80145AA8[0] & 0x5000);

  if (D_80145AA8[0] & D_80145AC2) {
    offset = D_80145FCC[D_801D428A * 0x140] * 164;
    /* The two ownership comparisons below are the two ways into the shared
     * cue-0x107 path (owner matches with the flag clear, owner differs with the
     * flag set); the compiler loads the selected record's owner byte once and
     * folds the pair into the original's single owner test followed by the flag
     * test, so the cue-0x104 path stays the sole else arm. */
    if ((D_80144983[offset] == masterIndex && value == 0) ||
        (D_80144983[offset] != masterIndex && value != 0)) {
      game_queue_frontend_cue(0x107);
    } else {
      game_queue_frontend_cue(0x104);
      D_801D4284 = 0;
      D_801D4286 = D_801D4286 + 1;
    }
  } else if (D_80145AA8[0] & D_80145AC4) {
    D_801D4284 = 1;
    game_queue_frontend_cue(0x102);
    game_queue_frontend_cue(0x106);
    D_801D4286 = 4;
  } else if (direction & 0x1000) {
    if (D_801D428A != 0) {
      D_801D428A = D_801D428A - 1;
    } else {
      D_801D428A = D_80146254 - 1;
    }
    game_queue_frontend_cue(0x100);
  } else if (direction & 0x4000) {
    if (D_801D428A == D_80146254 - 1) {
      D_801D428A = 0;
    } else {
      D_801D428A = D_801D428A + 1;
    }
    game_queue_frontend_cue(0x100);
  }

  for (index = 0; index < D_80146254; index++) {
    func_801D177C(0x11, 0x3E + index * 0x36, D_80145FCC[index * 0x140], index);
    func_801D1444(0x89, 0x3E + index * 0x36, D_80145FCC[index * 0x140]);
  }

  func_801D1CC0(0x11, (D_801D428A * 0x36 + 0x3E) & 0xFFFE, 0x110, 0x34, 1, 6);
  func_801D1E54(0x14, 0x10, 0x118, 0x13, D_80144952);
  func_8014F800(0x1B, 0x13, 0, 0xFF,
                0x80010000u + ((u16*)0x80010000)[action & 0xFFFF]);
}
