#include "bof3/world/area03004_internal.h"

void func_801D7B64(void);

/* @source 0x801D4B60
 * @behavior draws the selected entry panels at fixed coordinates, using work
 * halfword 0x3E and its entry-amount byte when available or the 0xFF/0xFFFF
 * sentinels otherwise. Draws the entry number, header and mode indicators.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSelectedEntryPanelsWorld00Area03004_801D4B60(void) {
  u8* entry;
  u8* sprite;
  u8* texture;
  u8* mode;
  s16 index;
  u32 amount;
  u32 mask;

  submitPanelPair(0, 78);
  entry = D_1F800044;
  index = FIELD_REF(s16, entry, 0x3E);
  amount = D_80144FE4[index];
  if (amount != 0) {
    func_801E1004((u8)index, amount, 0);
    sprite = D_1F800044;
    configureSpriteClut(128, 91, sprite[0x3E]);
    texture = D_1F800044;
    func_801E162C(8, 148, FIELD_REF(u16, texture, 0x3E));
  } else {
    func_801E1004(255, 0, 0);
    configureSpriteClut(128, 91, 255);
    func_801E162C(8, 148, 0xFFFF);
  }
  func_801E1320(176, 156);
  func_801D7B64();
  func_801D9534(20, 18, 280, 19, 0);
  mask = 0;
  if (D_80145E93 >= 3) {
    mode = D_1F800044;
    mask = (u8)(1u << mode[6]);
  }
  drawLabeledStatusCellsWorld00Area03004_801D7C10(88, 40, mask);
}
