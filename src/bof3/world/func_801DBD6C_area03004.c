#include "bof3/world/area03004_internal.h"

/* @source 0x801DBD6C
 * @behavior AREA030 panel step: decrements byte 9 of the work record published
 * at the scratchpad cursor 0x1F800044, submits the panel pair at y 78,
 * configures the sprite at 0x80 with CLUT 91 and byte 6 of the active 0x98-byte
 * work record D_80146888[D_801E3208], draws that record byte together with the
 * record byte at +0x90 through func_801E1004, draws the icon row at (176, 156)
 * and, while the dispatch byte 0xB reads 1 or 2, sprite 8 at 148 + 30 *
 * remaining count with image 67 (state 1) or 66 (state 2) through
 * func_801E162C; once the count expired it either clears byte 6 and stores 0x0C
 * into dispatch byte 3 (state 1) or advances dispatch byte 3 (state 2); finally
 * runs func_8014D978 and appends the dim tile.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DBD6C(void) {
  u8* work;

  work = D_1F800044;
  work[9] = work[9] - 1;
  submitPanelPair(0, 78);
  configureSpriteClut(0x80, 91, D_80146888[D_801E3208].unk_02[4]);
  func_801E1004(D_80146888[D_801E3208].unk_02[4],
                D_80146888[D_801E3208].unk_60[0x30], 0);
  func_801E1320(176, 156);
  work = D_1F800044;
  if (work[0xB] == 1) {
    func_801E162C(8, work[9] * 30 + 148, 67);
  } else if (work[0xB] == 2) {
    func_801E162C(8, work[9] * 30 + 148, 66);
  }
  work = D_1F800044;
  if (work[9] == 0) {
    if (work[0xB] == 1) {
      work[6] = 0;
      work = D_1F800044;
      work[3] = 0x0C;
    } else if (work[0xB] == 2) {
      work[3]++;
    }
  }
  func_8014D978();
  appendDimTile();
}
