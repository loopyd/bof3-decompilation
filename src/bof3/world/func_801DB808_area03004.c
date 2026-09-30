#include "bof3/world/area03004_internal.h"

/* @source 0x801DB808
 * @behavior AREA030 panel step: decrements byte 9 of the work record published
 * at the scratchpad cursor 0x1F800044, submits the panel pair at y 78,
 * configures the sprite at 0x80 with CLUT 91 and byte 6 of the active 0x98-byte
 * work record D_80146888[D_801E3208], draws that same record byte together with
 * the remaining count through func_801E1004, advances scratch byte 3 when the
 * count expired, then runs func_8014D978 and appends the dim tile.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DB808(void) {
  u8* work;

  work = D_1F800044;
  work[9] = work[9] - 1;
  submitPanelPair(0, 78);
  configureSpriteClut(0x80, 91, D_80146888[D_801E3208].unk_02[4]);
  func_801E1004(D_80146888[D_801E3208].unk_02[4], 0, D_1F800044[9]);
  work = D_1F800044;
  if (work[9] == 0) {
    work[3]++;
  }
  func_8014D978();
  appendDimTile();
}
