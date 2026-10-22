#include "bof3/battle/battle15_internal.h"

/* The panel-task root cell at 0x80148648 is read here through its symbol name:
 * each access then materializes the cell address separately, matching the
 * original bytes. The raw-constant g_PanelTaskRoot macro the header defines for
 * that same name would let the compiler keep the address in a callee-saved
 * register across the draw calls that follow. The record itself is wider than
 * PanelTask: this function also reads the size bytes at +0x08/+0x09. */
#undef D_80148648
/* The header declares the cell as PanelTask*, which stops at +0x08; this
 * function also reads the size bytes at +0x08/+0x09, so it views the same cell
 * through BattlePanelBoxTask. */
#define BATTLE_PANEL_BOX ((BattlePanelBoxTask *)D_80148648)

/* @source 0x800B0498
 * @behavior Draws the battle local panel: the box pass for the panel origin
 *   and size, three shaded rectangle regions, the AP label and the formatted
 *   battler index, then the cursor pass for the active selection substate. The
 *   label index count is 7 while the active message slot's +0x8A word is below
 *   the 0x801463B8 selection byte and 0 otherwise.
 * @status partial
 * @match 85.43
 * @residual 129/151 instructions; 604 original bytes versus 600 current.
 * The selection-substate tail uses different argument register homes and
 * scheduling. The cursor call keeps the fixed declaration; its callee ignores
 * incoming registers beyond the coordinates.
 */
void func_800B0498(void)
{
  u16 panel_count;
  s8 substate;
  u16 cursor_x;
  u16 cursor_y;
  u8 *bank;
  u8 *sel;

  func_800B06F4((s16)BATTLE_PANEL_BOX->x, (s16)BATTLE_PANEL_BOX->y,
                BATTLE_PANEL_BOX->width, BATTLE_PANEL_BOX->height);

  bank = &D_80144952;
  func_801AE3F0((u16)(BATTLE_PANEL_BOX->x + 6),
                (u16)(BATTLE_PANEL_BOX->y + 0x21),
                (u16)((BATTLE_PANEL_BOX->width << 3) - 0xF),
                (u16)((BATTLE_PANEL_BOX->height << 3) - 0x27), 0,
                *bank);
  func_801AE3F0((u16)(BATTLE_PANEL_BOX->x + 0x36),
                (u16)(BATTLE_PANEL_BOX->y + 6), 0x48, 0x18, 0,
                *bank);
  func_801AE3F0((u16)(BATTLE_PANEL_BOX->x + 0x87),
                (u16)(BATTLE_PANEL_BOX->y + 0xA), 0x30, 0x10, 0,
                *bank);

  sel = &D_801463B8;
  panel_count = (PSX_REF(u16, (u32)D_801EBF08 + 0x8Au) < *sel) ? 7 : 0;
  func_8014FF0C((s16)(BATTLE_PANEL_BOX->x + 0x88),
                (s16)(BATTLE_PANEL_BOX->y + 0xC), panel_count,
                &battleApParamStrings);
  sprintf((char *)D_80145AD4, (const char *)&battlePanelIndexFormat,
          *sel);
  func_8014FF0C((s16)(BATTLE_PANEL_BOX->x + 0xA0),
                (s16)(BATTLE_PANEL_BOX->y + 0xC), panel_count,
                D_80145AD4);

  /* The 0x801462E3 substate mode is read through a non-volatile view here: the
   * header's volatile declaration makes the 4 test emit an extra
   * `andi v0,v0,0xff` that the original bytes do not carry. */
  if (PSX_REF(u8, 0x801462E3u) == 4) {
    substate = (s8)D_801462E6;
    if (substate == (s8)-1) {
      cursor_x = (u16)(BATTLE_PANEL_BOX->x + 0xE);
      cursor_y = (u16)(BATTLE_PANEL_BOX->y + 0xC);
    } else {
      cursor_x = (u16)(BATTLE_PANEL_BOX->x + 0xE +
                       PSX_REF(u8, 0x801462E7u) * 0x1E);
      cursor_y = (u16)(BATTLE_PANEL_BOX->y + 0x2C + ((u8)substate << 5));
    }
    func_801647C4(cursor_x, cursor_y, 0);
  }
}
