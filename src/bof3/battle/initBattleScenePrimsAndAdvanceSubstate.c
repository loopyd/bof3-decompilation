#include "bof3/bof3.h"

/* Shared battle RAM: 0x801462E3 comes from config/targets/shared/symbols.txt,
 * the other two cells from this target's map. 0x80143C40 is the scene gate
 * halfword the other battle overlays test, 0x801462E3 is the dispatch substate
 * byte of this overlay's own table, and 0x801463B0 is the halfword battle/03
 * fills from the D_80181EB8 entry before func_80161FDC; the role of the 0x2C
 * published here is not established. */
extern u16 D_80143C40; /* @source 0x80143C40 @kind unknown */
extern u8 D_801462E3;  /* @source 0x801462E3 @kind unknown */
extern u16 D_801463B0; /* @source 0x801463B0 @kind unknown */

void func_801EF318(void);
void func_801EF180(u8 color);

/* @source 0x801EEEB8
 * @behavior Rebuilds the battle scene draw environment and its 0x80 sprite
 *           through this target's two setup routines, then, while the shared
 *           scene gate D_80143C40 stays clear, publishes 0x2C to the battle
 *           halfword D_801463B0 and advances the overlay dispatch substate
 *           byte D_801462E3.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initBattleScenePrimsAndAdvanceSubstate(void) {
  u8 substate;

  func_801EF318();
  func_801EF180(0x80);
  if (D_80143C40 == 0) {
    substate = D_801462E3;
    D_801463B0 = 0x2C;
    D_801462E3 = substate + 1;
  }
}
