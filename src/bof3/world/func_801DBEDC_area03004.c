#include "bof3/world/area03004_internal.h"

/* @source 0x801DBEDC
 * @behavior AREA030 panel step: submits the panel pair at y 78, configures the
 * sprite at 0x80 with CLUT 91 and byte 6 of the active 0x98-byte work record
 * D_80146888[D_801E3208], draws that record byte together with the record byte
 * at +0x90 through func_801E1004, draws the icon row at (176, 156) and sprite
 * 66 at (8, 148), then, while the shared pad mask D_80145AA8/0x50 is set,
 * stores 0x0E into dispatch byte 3 of the scratch work-record cursor at
 * 0x1F800044; finally runs func_8014D978 and appends the dim tile.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DBEDC(void) {
  submitPanelPair(0, 78);
  configureSpriteClut(0x80, 91, D_80146888[D_801E3208].unk_02[4]);
  func_801E1004(D_80146888[D_801E3208].unk_02[4],
                D_80146888[D_801E3208].unk_60[0x30], 0);
  func_801E1320(176, 156);
  func_801E162C(8, 148, 66);
  if ((D_80145AA8 & 0x50) != 0) {
    /* The fixed-address scratch cursor view, not the named D_1F800044 symbol:
     * only this representation reproduces the original's constant-in-delay-slot
     * store schedule (li v0,14 before the cursor load). */
    WORLD00_AREA030_SCRATCH_PTR[3] = 0x0E;
  }
  func_8014D978();
  appendDimTile();
}
