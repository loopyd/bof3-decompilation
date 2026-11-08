#include "bof3/bof3.h"

/* Battle mode byte the main program publishes at 0x801462EA: the shared battle
 * code selects the mode handler table entry with it and the battle overlays of
 * this archive family read it back as the current mode. */
extern u8 D_801462EA; /* @source 0x801462EA @kind unknown */

/* Acting battler byte at 0x80146374: the shared battle code indexes the battler
 * record arrays with it, so the overlay handlers treat it as the acting record
 * index. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* @source 0x800C1C4C
 * @behavior Step handler of this overlay, named by the single data word at
 * 0x800C2690 -- two words before the handler run the dispatcher at 0x800C1CC4
 * indexes at 0x800C2698 -- inside this payload's 0x800C25D8 data row; no jal in
 * the payload reaches it, so the loader that steps that row calls it. When the
 * battle mode byte 0x801462EA is 0x10 and the acting battler byte 0x80146374 is
 * 8, it raises the signed halfword +0xF8 of the current enemy work record
 * published at 0x801EB4E8 from zero to 1; that halfword is left untouched
 * otherwise, and the second condition is not even read unless the mode matches.
 * It then raises the unsigned halfword +0x94 of the same record from zero to 1
 * on every path, leaving it untouched when it is already nonzero. The second
 * statement re-reads 0x801EB4E8 instead of reusing the pointer loaded for the
 * first one wherever the first statement stored, because the record store may
 * alias that cell; on the paths that skip the store the loaded pointer is
 * reused, which is why the original loads the cell once at 0x800C1C78 and once
 * more at 0x800C1C98. The function is a leaf with no frame: it takes no
 * arguments, returns nothing and keeps no state across the two stores. Roles of
 * both halfwords and of mode 0x10 are not established here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseWorkHalfwordF8OnBattleMode10AndActingBattler8ThenRaiseWorkHalfword94BossBoss01316_800C1C4C(void) {
  if (D_801462EA == 0x10u && D_80146374 == 8u) {
    if (FIELD_REF(s16, D_801EB4E8, 0xF8u) == 0) {
      FIELD_REF(s16, D_801EB4E8, 0xF8u) = 1;
    }
  }
  if (FIELD_REF(u16, D_801EB4E8, 0x94u) == 0) {
    FIELD_REF(u16, D_801EB4E8, 0x94u) = 1;
  }
}
