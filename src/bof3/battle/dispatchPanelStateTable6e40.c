#include "bof3/battle/battle15_internal.h"

/* Use the named root cell so the post-dispatch access reloads its address. */
#undef D_80148648

extern BattleSelectionHandler D_800B6E40[]; /* @source 0x800B6E40 @kind table */
extern u8 D_80144F5D[]; /* @source 0x80144F5D @kind data */
extern u8 D_80181B10[]; /* @source 0x80181B10 @kind data */
void func_800B3324(u8 index, u16 x, u16 y, u32 field_20, u8 flags,
                   void *panel);

/* @source 0x800B232C
 * @behavior Dispatches the panel state through the table at 0x800B6E40, then
 * forwards the mapped byte at panel +0x0C, coordinates, +0x20 word, +0x0D byte
 * and panel pointer to func_800B3324.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPanelStateTable6e40(void) {
    PanelTask *panel;

    D_800B6E40[D_80148648->state]();
    panel = D_80148648;
    func_800B3324(D_80181B10[D_80144F5D[FIELD_REF(u8, panel, 0x0C)]],
                  panel->x, panel->field_06, FIELD_REF(u32, panel, 0x20),
                  FIELD_REF(u8, panel, 0x0D), panel);
}
