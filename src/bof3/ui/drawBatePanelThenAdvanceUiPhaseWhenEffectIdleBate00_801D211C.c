#include "bof3/bof3.h"

/* Shared front-end effect-busy gate halfword of the main executable, read here
 * with a zero-extending halfword load while the panel pair is already drawn.
 * It lies outside this EMI payload and keeps the original `lhu` width.
 * @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;

/* Shared sprite-grid index byte of the main executable, handed straight to the
 * overlay sprite emitter func_801D627C. The original reads it with `lbu` into
 * the first argument register ahead of the frame setup.
 * @source 0x80144953 @kind unknown */
extern u8 D_80144953;

/* Shared UI phase byte of the main executable: the index this overlay's
 * three-entry phase handler table is dispatched with. Owned by the shared map.
 * @source 0x80148651 @kind unknown */
extern u8 D_80148651;

/* Shared UI sub-step byte of the main executable, the index the sub-step
 * dispatcher of this overlay reads and the one cleared here on the phase
 * change. Owned by the shared map.
 * @source 0x80148652 @kind unknown */
extern u8 D_80148652;

/* Shared cursor-kind byte of the main executable, which the sibling handler
 * func_801D2190 bounds by six before indexing this overlay's D_801D8F00 state
 * table and which is cleared here on the phase change. The generated
 * undefined_syms_auto map already owns this address, so no target-map row is
 * added.
 * @source 0x80148658 @kind unknown */
extern u8 D_80148658;

/* Sprite emitter of this overlay: takes the shared sprite-grid index as its
 * only argument and returns nothing this body consumes.
 * @source 0x801D627C @kind unknown */
extern void func_801D627C(u32 grid);

/* Panel emitter of this overlay: draws the panel anchored at the signed
 * coordinates the two arguments hold. Its own prologue broadens the pair with
 * `andi 0xFFFF`, so the parameters are declared wide here and the constant
 * arguments keep their bare `addiu` form.
 * @source 0x801D2304 @kind unknown */
extern void func_801D2304(s32 x, s32 y);

/* State-table reset of this overlay, called with no arguments once the
 * effect-busy gate has cleared. Its own span zeroes the six words of the
 * D_801D8F00 table, so nothing is returned or read back here.
 * @source 0x801D259C @kind unknown */
extern void func_801D259C(void);

/* @source 0x801D211C
 * @behavior Second handler of the BATE overlay sub-step table D_801D8770,
 *           which dispatchUiSubStepTableBate00_801D20AC indexes with the shared
 *           UI sub-step byte, and the same drawing pair its sibling handler
 *           func_801D2190 opens with. Every visit first redraws the shared
 *           screen: it passes the shared sprite-grid index byte D_80144953 to
 *           this overlay sprite emitter func_801D627C and then draws this
 *           overlay panel at the fixed coordinates 0x37, 0x37 through
 *           func_801D2304. It then reads the shared front-end effect-busy gate
 *           halfword D_80143C40 once and, while that gate is set, returns with
 *           the redraw as its whole effect. Once the gate is clear it resets
 *           this overlay's state table through func_801D259C, clears the
 *           shared cursor-kind byte D_80148658 and the shared UI sub-step byte
 *           D_80148652, and advances the shared UI phase byte D_80148651 by
 *           one, which sends the next dispatch to the following phase. Nothing
 *           is returned and the 0x18-byte frame only keeps the return address
 *           at 0x10 across the three calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawBatePanelThenAdvanceUiPhaseWhenEffectIdleBate00_801D211C(void) {
  u8 phase;

  func_801D627C(D_80144953);
  func_801D2304(0x37, 0x37);
  if (D_80143C40 != 0) {
    return;
  }
  func_801D259C();
  phase = D_80148651;
  D_80148658 = 0;
  D_80148652 = 0;
  D_80148651 = phase + 1;
}
