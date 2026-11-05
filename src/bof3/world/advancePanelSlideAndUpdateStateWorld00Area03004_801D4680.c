#include "bof3/world/area03004_internal.h"

/* @source 0x801D4680
 * @behavior increments work byte 9 and draws the AREA030 panels at sliding
 * offsets derived from that byte. Work byte 0xB selects the fixed or sliding
 * header; byte 7 selects the right panel variant. On step four, clears state
 * byte 1 when byte 0xB is zero, otherwise advances it for mode 1 or sets it to
 * 12 for mode 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advancePanelSlideAndUpdateStateWorld00Area03004_801D4680(void) {
  u8* step;
  u8* header;
  u8* left;
  u8* mode;
  u8* middle;
  u8* right;
  u8* state;
  u32 mask;

  step = D_1F800044;
  step[9]++;
  header = D_1F800044;
  if (header[0xB] == 0) {
    func_801D9534(20, (18 - header[9] * 16) & 0xFFFE, 280, 19, 0);
    left = D_1F800044;
    drawLabeledStatusCellsWorld00Area03004_801D7C10((u16)(88 + left[9] * 45), 40, 0);
  } else {
    func_801D9534(20, 18, 280, 19, 0);
    mask = 0;
    if (D_80145E93 >= 3) {
      mode = D_1F800044;
      mask = (u8)(1u << mode[6]);
    }
    drawLabeledStatusCellsWorld00Area03004_801D7C10(88, 40, mask);
  }
  middle = D_1F800044;
  func_801D88E0((s16)(16 - middle[9] * 45), 64);
  right = D_1F800044;
  if ((right[7] & 1) == 0) {
    func_801D7EB0((s16)(164 + right[9] * 45), 64);
  } else {
    func_801D8364((s16)(164 + right[9] * 45), 64);
  }
  state = D_1F800044;
  if (state[9] == 4) {
    if (state[0xB] == 0) {
      state[1] = 0;
    } else if (state[6] == 1) {
      state[1]++;
    } else if (state[6] == 2) {
      state[1] = 12;
    }
  }
}
