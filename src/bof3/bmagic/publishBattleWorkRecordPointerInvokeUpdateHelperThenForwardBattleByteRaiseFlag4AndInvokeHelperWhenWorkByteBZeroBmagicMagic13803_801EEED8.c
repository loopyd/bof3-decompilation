#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_8014D978(void);
void func_801E5988(void);

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The original re-materialises the cell address with its own
 * lui for the publish store and again for the restoring store, and loads it once
 * into $s0 before the publish, so the accesses go through the named pointer
 * rather than a folded constant base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * The original loads the pointer it holds and publishes that as the work object,
 * so the cell is a pointer cell rather than a byte record. This target's
 * generated undefined-syms map already binds the address because the overlay's
 * remaining assembly spans load it at 0x2A0 and 0x478, so the name needs no
 * target-map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800A9820 consumes. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8. Bit 0x4 is raised through an
 * unsigned halfword load, an ori and a halfword store whose base is
 * re-materialised into $at. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEED8
 * @behavior Overlay header handler, entry index 2 of the nine-word handler
 *           pointer run at 0x801EEC04 (this entry's payload word is at
 *           0x801EEC0C). The run's only in-payload reader is the dispatcher at
 *           0x801EEC28, which copies the first three entries into its own frame
 *           and indexes them with the work object's byte 0x1, so this entry is
 *           invoked with no arguments when that byte is 2. The body publishes
 *           the shared battle work record pointer held at 0x801EB4E0 into the
 *           scratchpad work-object pointer cell at 0x1F800044 while keeping the
 *           cell's previous value in a callee-saved register, invokes the shared
 *           update helper at 0x8014D978 for its effect only, restores the cell,
 *           and then tests byte 0xB of the restored work object. Only when that
 *           byte is zero does it load the shared battle byte at 0x80146384
 *           unsigned, pass it to the shared helper func_800A9820, raise bit 0x4
 *           of the shared battle global halfword at 0x801462E8 and invoke the
 *           shared helper func_801E5988. It returns nothing and writes no other
 *           state; the 0x18-byte frame keeps $ra at 0x14 and the saved cell value
 *           at 0x10 across the two calls, and the 0x801EB4E0 load is scheduled
 *           before the frame is created because it needs no stack. The roles of
 *           work byte 0xB, work byte 0x1 and the global record are not
 *           established, so they stay offset-anchored. The 0x88-byte span was
 *           compared against every payload under out/binaries when this body was
 *           written and occurs only here, so no clone seed existed; the
 *           statement order, the publish-and-restore block and the
 *           forward-raise-invoke tail were taken from the exact sibling
 *           publishSavedWorkPointerInvokeReadinessHelperThenForwardBattleByte
 *           RaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic08103_801EEF88,
 *           whose span differs only in that it loads record word 0x74 and calls
 *           0x8014DAEC, and from this target's still-assembly spans at 0x801EEC88
 *           and 0x801EED60, which carry the same publish-and-restore block.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleWorkRecordPointerInvokeUpdateHelperThenForwardBattleByteRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic13803_801EEED8(
    void) {
  u8 *saved;

  saved = g_battle_work;
  g_battle_work = D_801EB4E0;
  func_8014D978();
  g_battle_work = saved;
  if (saved[0xB] == 0) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
