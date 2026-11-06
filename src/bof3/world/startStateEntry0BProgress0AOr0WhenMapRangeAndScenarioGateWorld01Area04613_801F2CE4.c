#include "bof3/bof3.h"

/* Shared state-entry selector: dispatchIndexedStateHandler indexes the
 * D_801C84BC handler table with this byte, and the world overlays publish
 * their next state entry here. The original stores it with one `sb`.
 * @source 0x801448EB @kind unknown */
extern u8 D_801448EB;
/* Progress byte of the shared state-entry tuple 0x801448EB..0x801448ED, stored
 * with one `sb`.
 * @source 0x801448EC @kind unknown */
extern u8 D_801448EC;
/* Shared area status byte, stored with one `sb`.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;
/* Signed leading byte of the shared scenario-state record, which etc/game/00
 * declares as scenarioState. The original takes it with `lb` and tests it
 * against 8, so the cell keeps that signed byte width.
 * @source 0x80146870 @kind bss */
extern s8 D_80146870;

u32 func_80166140(u32 arg0);
void func_8015C088(void);

/* @source 0x801F2CE4
 * @behavior Overlay gate that starts the shared state entry 0xB. It rejects a
 *   second argument above 0x218000, a first argument whose high half is
 *   outside 0xE..0x10 and a nonzero shared selector func_80166140(4); on pass it
 *   runs the shared front-end startup helper func_8015C088, stores 1 into the
 *   shared area status byte D_80146865 and 0xB into the shared state-entry
 *   selector D_801448EB, then stores 0 into the progress byte D_801448EC while
 *   the signed scenario byte D_80146870 is below 8 and 0xA once it reaches 8,
 *   returning 1 from either arm. Every rejected path returns 0. The 0x18-byte
 *   frame only keeps $ra across the two calls, and every byte store
 *   materialises the 0x8014 page in $at on its own.
 *
 * Name evidence: 0x801448EB is the state selector that etc/game/00 dispatches
 *   its D_801C84BC handler table through and that the world overlays set as
 *   their state entry; 0x801448EC is the progress byte of the same three-byte
 *   tuple (initArea14613State writes 0x2C/0/4 to it and its neighbours);
 *   0x80146865 is the shared area status byte the world overlays clear and set;
 *   0x80146870 is the signed leading byte of the etc/game/00 scenario-state
 *   record, declared s8 at its original `lb` width in
 *   include/bof3/ui/shop00_internal.h. Neither argument carries an evidenced
 *   role, so the name keeps them structural.
 *
 * Shape: seeded from this target's original bytes with the exact sibling
 *   startStateEntry0DProgress0WhenMapRangeAndModeGateWorld02Area09713_801F2C98
 *   (0x801F2C98) for the reject ladder - `arg1` against its limit, `(u32)((arg0
 *   >> 16) - base) >= 3u` and `func_80166140(k) != 0`, then func_8015C088 and
 *   the state-tuple writes - and from
 *   selectAreaStateForMatchedArgumentsWorld04Area17613_801F3378 (0x801F3378)
 *   for the `lb` + `slti 8` scenario gate. No byte-identical twin exists: the
 *   164-byte span and its discriminating immediate 0x218000 occur only in this
 *   payload, so the clone lever is ineligible.
 *
 *   The three reject paths are one conjunction around the accepted block rather
 *   than a ladder of early returns. Written as three early `return 0`
 *   statements the same body measured 33/41 at an unchanged 164 bytes with
 *   first=+0x0010: the merged reject block was laid out before the second call
 *   and `bnez` on the selector test branched over it, so the reject block
 *   occupied the slot the original keeps for the func_8015C088 call and the
 *   scenario arms swapped places. Sharing one trailing `return 0` puts the
 *   three rejects on the single block the original keeps after the scenario
 *   arms, which also restores the `beqz`/`bnez` polarities and both arm jumps.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 startStateEntry0BProgress0AOr0WhenMapRangeAndScenarioGateWorld01Area04613_801F2CE4(
    s32 arg0, s32 arg1) {
  if (arg1 <= 0x218000 && (u32)((arg0 >> 16) - 0xE) < 3u &&
      func_80166140(4) == 0) {
    func_8015C088();
    D_80146865 = 1;
    D_801448EB = 0xB;
    if (D_80146870 < 8) {
      D_801448EC = 0;
      return 1;
    }
    D_801448EC = 0xA;
    return 1;
  }
  return 0;
}
