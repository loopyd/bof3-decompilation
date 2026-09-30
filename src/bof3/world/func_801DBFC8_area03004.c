#include "bof3/world/area03004_internal.h"

/* @source 0x801DBFC8
 * @behavior AREA030 panel step: submits the panel pair at y 78, configures the
 * sprite at 0x80 with CLUT 91 and byte 6 of the active 0x98-byte work record
 * D_80146888[D_801E3208], draws that record byte together with the record byte
 * at +0x90 through func_801E1004, draws the icon row at (176, 156) and sprite
 * 67 at (8, 148), then, while the shared pad flag D_80145AA8 is set, clears
 * byte 6 of the scratch work-record cursor at 0x1F800044 and advances its
 * dispatch byte 3; finally runs func_8014D978 and appends the dim tile.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DBFC8(void) {
  submitPanelPair(0, 78);
  configureSpriteClut(0x80, 91, D_80146888[D_801E3208].unk_02[4]);
  func_801E1004(D_80146888[D_801E3208].unk_02[4],
                D_80146888[D_801E3208].unk_60[0x30], 0);
  func_801E1320(176, 156);
  func_801E162C(8, 148, 67);
  if (D_80145AA8 != 0) {
    D_1F800044[6] = 0;
    D_1F800044[3]++;
  }
  func_8014D978();
  appendDimTile();
}
