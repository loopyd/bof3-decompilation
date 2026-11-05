#include "bof3/world/area03004_internal.h"

/* @source 0x801DB8F0
 * @behavior AREA030 selection-panel step handler, entry 7 of the local
 *           work-record step table 0x801E2268: while the shared pad state
 *           D_80145AA8 carries the 0x40 bit it reloads byte 9 of the work
 *           record published at the scratchpad cursor 0x1F800044 from the
 *           halfword at +0x90 of the active 0x98-byte work record
 *           D_80146888[D_801E3208], otherwise it advances that byte by one;
 *           it then submits the panel pair at y 78, configures the sprite at
 *           0x80 with CLUT 91 and byte 6 of the active record, draws that
 *           record byte together with the countdown through func_801E1004,
 *           and once the countdown equals the record halfword at +0x90 it
 *           plays cue 0x20D while the D_801E3214 flag byte is set, plays cue
 *           0x102, restarts the countdown at 4 and advances scratch byte 3;
 *           finally it runs func_8014D978 and appends the dim tile.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The two single-access countdown updates read the published D_1F800044
 * symbol directly, while the groups that write and then advance the same
 * record keep the reassigned `work` local: the direct reads keep the reload
 * behind the record index arithmetic the original shows, where a pointer
 * local assigned first fills that slot and loses the following load-delay
 * nop.
 */
void restartPanelCountdownOnPadHoldAndPlayCueWorld00Area03004_801DB8F0(void) {
  u8* work;

  if (D_80145AA8 & 0x40) {
    D_1F800044[9] = *(u16*)&D_80146888[D_801E3208].unk_60[0x30];
  } else {
    work = D_1F800044;
    work[9] = work[9] + 1;
  }
  submitPanelPair(0, 78);
  configureSpriteClut(0x80, 91, D_80146888[D_801E3208].unk_02[4]);
  func_801E1004(D_80146888[D_801E3208].unk_02[4], D_1F800044[9], 0);
  if (D_1F800044[9] == *(u16*)&D_80146888[D_801E3208].unk_60[0x30]) {
    if (D_801E3214) {
      func_8015DF18(0x20D);
    }
    func_8015DF18(0x102);
    work = D_1F800044;
    work[9] = 4;
    work = D_1F800044;
    work[3]++;
  }
  func_8014D978();
  appendDimTile();
}
