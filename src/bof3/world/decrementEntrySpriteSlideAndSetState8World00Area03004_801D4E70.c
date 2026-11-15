#include "bof3/world/area03004_internal.h"

void func_801D7B64(void);

/* @source 0x801D4E70
 * @behavior decrements work byte 9, draws the entry panels and slides the
 * selected entry sprite right or left according to work byte 8. Uses the
 * 0xFF sentinel for an unavailable entry, draws the fixed header and mode
 * indicators, and sets state byte 1 to 8 when the sliding byte reaches zero.
 * @status partial
 * @match 93.22
 * @residual countdown and sprite index register allocation and initial
 * sentinel argument setup differ from the original
 */
void decrementEntrySpriteSlideAndSetState8World00Area03004_801D4E70(void) {
  u8* step;
  u8* entry;
  u8* sprite;
  u8* mode;
  u8* state;
  s16 index;
  u8 entry_arg;
  u32 amount;
  u32 mask;

  step = D_1F800044;
  index = (s16)(step[9] - 1);
  step[9] = (u8)index;
  submitPanelPair(0, 78);
  entry = D_1F800044;
  index = FIELD_REF(s16, entry, 0x3E);
  amount = D_80144FE4[index];
  entry_arg = (u8)index;
  if (amount == 0) {
    entry_arg = 255;
  }
  func_801E1004(entry_arg, amount, 0);
  sprite = D_1F800044;
  if (sprite[8] == 0) {
    index = FIELD_REF(s16, sprite, 0x3E);
    if (D_80144FE4[index] != 0) {
      configureSpriteClut((s16)(128 + sprite[9] * 48), 91, (u8)index);
    } else {
      configureSpriteClut((s16)(128 + sprite[9] * 48), 91, 255);
    }
  } else {
    index = FIELD_REF(s16, sprite, 0x3E);
    if (D_80144FE4[index] != 0) {
      configureSpriteClut((s16)(128 - sprite[9] * 48), 91, (u8)index);
    } else {
      configureSpriteClut((s16)(128 - sprite[9] * 48), 91, 255);
    }
  }
  func_801E1320(176, 156);
  func_801E162C(8, 148, 0xFFFF);
  func_801D7B64();
  WORLD00_AREA030_DRAW_HEADER_AND_STATUS(mode, mask);
  state = D_1F800044;
  if (state[9] == 0) {
    state[1] = 8;
  }
}
