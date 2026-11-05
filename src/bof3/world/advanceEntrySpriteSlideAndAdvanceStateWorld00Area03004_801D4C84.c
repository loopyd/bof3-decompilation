#include "bof3/world/area03004_internal.h"

void func_801D7B64(void);

/* @source 0x801D4C84
 * @behavior increments work byte 9 and slides the selected entry sprite left
 * or right according to work byte 8, using the 0xFF unavailable-entry sentinel.
 * Draws the entry panels, header and mode indicators. At count 4, copies work
 * halfword 0x3C to 0x3E and advances state byte 1.
 * @status partial
 * @match 79.67
 * @residual 32-byte frame instead of 56 bytes and work-pointer/index
 * register allocation differ from the original
 */
void advanceEntrySpriteSlideAndAdvanceStateWorld00Area03004_801D4C84(void) {
  u8* step;
  u8* entry;
  u8* sprite;
  u8* mode;
  u8* state;
  s32 index;
  u32 amount;
  u32 mask;

  step = D_1F800044;
  step[9]++;
  submitPanelPair(0, 78);
  entry = D_1F800044;
  index = FIELD_REF(s16, entry, 0x3E);
  amount = D_80144FE4[index];
  if (amount != 0) {
    func_801E1004((u8)index, amount, 0);
  } else {
    func_801E1004(255, 0, 0);
  }
  sprite = D_1F800044;
  if (sprite[8] == 0) {
    index = FIELD_REF(s16, sprite, 0x3E);
    if (D_80144FE4[index] != 0) {
      configureSpriteClut((s16)(128 - sprite[9] * 48), 91, (u8)index);
    } else {
      configureSpriteClut((s16)(128 - sprite[9] * 48), 91, 255);
    }
  } else {
    index = FIELD_REF(s16, sprite, 0x3E);
    if (D_80144FE4[index] != 0) {
      configureSpriteClut((s16)(128 + sprite[9] * 48), 91, (u8)index);
    } else {
      configureSpriteClut((s16)(128 + sprite[9] * 48), 91, 255);
    }
  }
  func_801E1320(176, 156);
  func_801E162C(8, 148, 0xFFFF);
  func_801D7B64();
  func_801D9534(20, 18, 280, 19, 0);
  mask = 0;
  if (D_80145E93 >= 3) {
    mode = D_1F800044;
    mask = (u8)(1u << mode[6]);
  }
  func_801D7C10(88, 40, mask);
  state = D_1F800044;
  if (state[9] == 4) {
    FIELD_REF(u16, state, 0x3E) = FIELD_REF(u16, state, 0x3C);
    state[1]++;
  }
}
