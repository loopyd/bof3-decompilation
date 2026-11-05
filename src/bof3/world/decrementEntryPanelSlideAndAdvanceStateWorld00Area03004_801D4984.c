#include "bof3/world/area03004_internal.h"

/* @source 0x801D4984
 * @behavior decrements work byte 9, draws the entry panels at offsets derived
 * from that byte, and uses work halfword 0x3E to select an available entry or
 * the 0xFF sentinel. Draws the fixed header and mode indicators, then advances
 * state byte 1 when the sliding byte reaches zero.
 * @status partial
 * @match 88.24
 * @residual 32-byte frame instead of 48 bytes and entry-selection register
 * allocation differ from the original
 */
void decrementEntryPanelSlideAndAdvanceStateWorld00Area03004_801D4984(void) {
  u8* step;
  u8* pair;
  u8* entry;
  u8* values;
  u8* icons;
  u8* texture;
  u8* mode;
  u8* state;
  s32 index;
  u32 mask;

  step = D_1F800044;
  step[9]--;
  pair = D_1F800044;
  submitPanelPair((s16)(-(pair[9] * 80)), 78);
  entry = D_1F800044;
  index = FIELD_REF(s16, entry, 0x3E);
  if (D_80144FE4[index] != 0) {
    configureSpriteClut((s16)(128 + entry[9] * 48), 91, (u8)index);
    values = D_1F800044;
    index = FIELD_REF(s16, values, 0x3E);
    func_801E1004((u8)index, D_80144FE4[index], values[9]);
  } else {
    configureSpriteClut((s16)(128 + entry[9] * 48), 91, 255);
    values = D_1F800044;
    func_801E1004(255, 0, values[9]);
  }
  icons = D_1F800044;
  func_801E1320(176, (s16)(156 + icons[9] * 30));
  texture = D_1F800044;
  func_801E162C(8, 148 + texture[9] * 30, 0xFFFF);
  func_801D9534(20, 18, 280, 19, 0);
  mask = 0;
  if (D_80145E93 >= 3) {
    mode = D_1F800044;
    mask = (u8)(1u << mode[6]);
  }
  func_801D7C10(88, 40, mask);
  state = D_1F800044;
  if (state[9] == 0) {
    state[1]++;
  }
}
