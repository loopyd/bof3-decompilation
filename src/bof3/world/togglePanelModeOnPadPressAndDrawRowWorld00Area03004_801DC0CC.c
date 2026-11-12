#include "bof3/world/area03004_internal.h"

/* @source 0x801DC0CC
 * @behavior AREA030 panel step: submits the panel pair at y 78, configures the
 * sprite at 0x80 with CLUT 91 and byte 6 of the active 0x98-byte work record
 * D_80146888[D_801E3208], draws that record byte together with the record byte
 * at +0x90 through func_801E1004, draws the icon row at (176, 156) and sprite
 * 68 at (8, 148), then reacts to the shared pad word D_80145AA8: while its
 * 0x5000 bits are set it plays cue 0x100 and inverts byte 6 of the scratch
 * work-record cursor at 0x1F800044; otherwise, while its 0x40 bit is set it
 * plays cue 0x103, and while its 0x10 bit is set it stores 1 in that byte and
 * plays cue 0x106; either of those arms then plays cue 0x102 and advances that
 * cursor dispatch byte 3. It then submits the row marker at (0x20, 0xB6 + 12 *
 * byte 6) through func_801647C4, runs func_8014D978 and appends the dim tile.
 * @status exact
 * @match 100.00
 * @residual none
 */
void togglePanelModeOnPadPressAndDrawRowWorld00Area03004_801DC0CC(void) {
  submitPanelPair(0, 78);
  configureSpriteClut(0x80, 91, D_80146888[D_801E3208].unk_02[4]);
  func_801E1004(D_80146888[D_801E3208].unk_02[4],
                D_80146888[D_801E3208].unk_60[0x30], 0);
  func_801E1320(176, 156);
  func_801E162C(8, 148, 68);
  if ((D_80145AA8 & 0x5000) != 0) {
    func_8015DF18(0x100);
    D_1F800044[6] ^= 1;
  } else if ((D_80145AA8 & 0x40) != 0) {
    func_8015DF18(0x103);
    func_8015DF18(0x102);
    D_1F800044[3]++;
  } else if ((D_80145AA8 & 0x10) != 0) {
    D_1F800044[6] = 1;
    func_8015DF18(0x106);
    func_8015DF18(0x102);
    D_1F800044[3]++;
  }
  func_801647C4(0x20, D_1F800044[6] * 12 + 0xB6, 0);
  func_8014D978();
  appendDimTile();
}
