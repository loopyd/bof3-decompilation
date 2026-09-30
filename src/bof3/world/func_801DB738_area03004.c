#include "bof3/world/area03004_internal.h"

/* @source 0x801DB738
 * @behavior AREA030 panel step: decrements byte 9 of the work record published
 * at the scratchpad cursor 0x1F800044, submits the panel pair at y 78,
 * configures the sprite at 0x80 + 0x30 * byte 9 with CLUT 91 and byte 6 of the
 * active work record D_80146888[D_801E3208], then, when that count reaches
 * zero, restarts it at 4 and advances dispatch byte 3 of the record; finally
 * runs func_8014D978 and appends the dim tile through appendDimTile.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DB738(void) {
  u8* work;

  work = D_1F800044;
  work[9] = work[9] - 1;
  submitPanelPair(0, 78);
  configureSpriteClut((s16)(D_1F800044[9] * 0x30 + 0x80), 91,
                      D_80146888[D_801E3208].unk_02[4]);
  work = D_1F800044;
  if (work[9] == 0) {
    work[9] = 4;
    work = D_1F800044;
    work[3]++;
  }
  func_8014D978();
  appendDimTile();
}
