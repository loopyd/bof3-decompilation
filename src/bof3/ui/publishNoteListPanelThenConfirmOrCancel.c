#include "bof3/ui/shop00_internal.h"

/* @source 0x801DFDF4
 * @behavior note-list sub-step handler reached as entry 0 of the sub-step table
 *           D_801E5CFC, which func_801DFDB8 dispatches with the UI sub-step byte
 *           D_80148652 as index: it first publishes the note-list panel state
 *           (the halfword D_80148340 = 0x36, the visibility byte D_80148624 = 1,
 *           the published x halfword D_80148628 = D_80148334 +
 *           (1 - D_8014865B) * 32 + 0xDE and the published y halfword
 *           D_8014862A = D_80148336 + 5), then plays cue 0x101 and toggles the
 *           work-block byte D_8014865B when the pad state D_80145AA8 shares a
 *           bit with 0xA000.
 *           When the pad state shares the confirm mask D_80145AC2 and that
 *           work-block byte is nonzero it plays cues 0x104 and 0x102, arms the
 *           list head counter D_801E625C with 0, calls func_801665A0 with
 *           (0, 0x58, 1, 0), and initializes the two 0x24-byte panel records
 *           anchored at D_80148378 with the field run (7, 0xD, 2, -150, 0x74)
 *           for the first record and (7, 0xC, 2, 0x140, 0x3E) for the second,
 *           carrying the first rebuilt list entry D_801E6264[D_801E625C] into
 *           both D_80148382 and the selected-record byte D_801483A6 while the
 *           companion bytes D_801483A7 and D_801483A9 stay zero and D_801483A8
 *           takes 0xFF; it then advances the UI sub-step byte D_80148652 by one.
 *           A confirm with that work-block byte clear, and the cancel mask
 *           D_80145AC4 on its own, both play cue 0x106 and step the UI phase
 *           byte D_80148651 down by one. The work-block byte is reached through
 *           a signed pointer because the original reads it with a signed byte
 *           load for the published x offset, while the other readers of the
 *           same byte use lbu.
 * @status partial
 * @match 82.98
 * @residual The confirm arm's panel-record stores retain scheduling and scratch
 *           register differences, beginning with li a0,7 versus li v1,7.
 *           The complete function size and 0x20 frame match; 24 of 141
 *           instructions differ.
 */
void publishNoteListPanelThenConfirmOrCancel(void) {
  u16* pad;
  s8*  flag;
  u8 firstEntry;

  D_80148340 = 0x36;
  D_80148624 = 1;
  flag = (s8*)&D_8014865B;
  pad = &D_80145AA8;
  D_80148628 = D_80148334 + (1 - *flag) * 32 + 0xDE;
  D_8014862A = D_80148336 + 5;

  if (*pad & 0xA000) {
    func_8015DF18(0x101);
    *flag ^= 1;
  }

  if (*pad & D_80145AC2) {
    if (*flag != 0) {
      func_8015DF18(0x104);
      func_8015DF18(0x102);
      D_801E625C = 0;
      func_801665A0(0, 0x58, 1, 0);
      D_80148379 = 7;
      D_8014837A = 0xD;
      D_8014837B = 2;
      D_80148378[0] = 1;
      firstEntry = D_801E6264[D_801E625C];
      D_8014839F = 2;
      D_8014837C = -150;
      D_8014837E = 0x74;
      D_8014839E = 0xC;
      D_8014839D = 7;
      D_8014839C = 1;
      D_80148382 = firstEntry;
      D_801483A8 = 0xFF;
      D_801483A0 = 0x140;
      D_801483A7 = 0;
      D_801483A9 = 0;
      D_801483A2 = 0x3E;
      D_801483A6 = D_801E6264[D_801E625C];
      D_80148652 = D_80148652 + 1;
      return;
    }
  } else if (!(*pad & D_80145AC4)) {
    return;
  }
  func_8015DF18(0x106);
  D_80148651 = D_80148651 - 1;
}
