#include "bof3/bof3.h"

/* Shared sprite-grid index byte of the main executable, handed straight to the
 * overlay sprite emitter func_801D627C. The original reads it with `lbu` into
 * the first argument register ahead of the frame setup. */
extern u8 D_80144953; /* @source 0x80144953 @kind unknown */

/* Shared pad-state cell of the main executable, of which only the low byte is
 * read here (`lbu`): a set low byte skips the rest of the current ramp slot and
 * ends the whole ramp sequence once every slot is done. */
extern u8 D_80145AA8; /* @source 0x80145AA8 @kind unknown */

/* Shared UI phase byte of the main executable, the index of this overlay's
 * three-entry phase handler table and the byte advanced here once every ramp
 * slot is done. Owned by the shared map. */
extern u8 D_80148651; /* @source 0x80148651 @kind unknown */

/* Shared cursor-kind byte of the main executable: the six-slot index this
 * handler ramps, cleared by the two sibling handlers of this overlay when the
 * phase changes. The bound test and both table indices read it as a signed
 * byte, the advance reads it unsigned. */
extern u8 D_80148658; /* @source 0x80148658 @kind unknown */

/* Per-slot ramp step bytes of this overlay (the shipped bytes at slots 0 to 5
 * are 1, 3, 3, 1, 1, 1). It sits inside the trailing data blob of the payload,
 * so the generated map already names it. */
extern u8 D_801D8778[]; /* @source 0x801D8778 @kind data */

/* Six per-slot ramp limits of this overlay, filled at runtime by func_801D259C
 * from the shared counters 0x801FE4E8, 0x801FE4EF and 0x801FE4F0. */
extern u32 D_801D8EE8[]; /* @source 0x801D8EE8 @kind data */

/* Six per-slot ramp values of this overlay, zeroed by func_801D259C and
 * stepped towards the limits above by this handler. */
extern u32 D_801D8F00[]; /* @source 0x801D8F00 @kind data */

/* Sprite emitter of this overlay: takes the shared sprite-grid index as its
 * only argument and returns nothing this body consumes. */
extern void func_801D627C(u32 grid); /* @source 0x801D627C @kind unknown */

/* Panel emitter of this overlay: draws the panel anchored at the signed
 * coordinates the two arguments hold. Its own prologue narrows the pair with
 * `andi 0xFFFF`, so the parameters are declared wide here and the constant
 * arguments keep their bare `addiu` form. */
extern void func_801D2304(s32 x, s32 y); /* @source 0x801D2304 @kind unknown */

/* @source 0x801D2190
 * @behavior Second handler of the BATE overlay phase handler table D_801D8764,
 *           reached once the sub-step phase advanced the shared UI phase byte,
 *           and the same drawing pair its two sibling handlers open with:
 *           every visit passes the shared sprite-grid index byte D_80144953 to
 *           this overlay sprite emitter func_801D627C and draws this overlay
 *           panel at the fixed coordinates 0x37, 0x37 through func_801D2304.
 *           It then ramps the six-slot panel values: the shared cursor-kind
 *           byte D_80148658 is the slot index, and while it is below six the
 *           slot's own step byte D_801D8778 raises its value D_801D8F00 by one
 *           of 1 or 3. A slot whose raised value has not yet reached its limit
 *           D_801D8EE8 returns unchanged while the pad low byte D_80145AA8 is
 *           clear; otherwise the value snaps to that limit and the slot index
 *           advances by one, which covers both a finished slot and a pad press
 *           that skips the rest of it. Once the slot index has reached six the
 *           handler only ends the sequence: a set pad low byte requests
 *           frontend-local mode 2 through func_8014ECAC and advances the shared
 *           UI phase byte D_80148651 by one, which hands the next dispatch to
 *           the third phase handler, and a clear one returns unchanged. The
 *           panel is not redrawn between slots and nothing is returned; the
 *           0x18-byte frame only keeps the return address at 0x10 across the
 *           redraw and, on the closing arm, the mode call.
 * @status partial
 * @match 56.06
 * @residual The original materializes the address of the cursor-kind byte once
 *           in $a2 right after the two calls and then reaches that byte three
 *           times at offset 0 of that register (`lb`, `lb` after the slot
 *           store, `lbu` before the slot advance), with the slot address
 *           recomputed from the reloaded index both times. This build folds all
 *           of those reads into one `lui`/`lb` pair and carries the value in an
 *           allocator copy across the branch, so it is two reads and the
 *           materialized base short. Measured shapes that force a reload each
 *           cost the frame eight bytes (0x20 against 0x18) or drop the
 *           materialized base, so the remaining lever is the compiler's
 *           cross-block copy of the entry-block load, not statement order,
 *           cast spelling or pointer local choice.
 */
void stepSixSlotRampAndAdvanceUiPhaseOnPadBate00_801D2190(void) {
  s8 *kindp;

  func_801D627C(D_80144953);
  func_801D2304(0x37, 0x37);
  kindp = (s8 *)&D_80148658;
  if (*kindp < 6) {
    D_801D8F00[*kindp] = D_801D8F00[*kindp] + D_801D8778[*kindp];
    if (D_801D8F00[*kindp] >= D_801D8EE8[*kindp] || D_80145AA8 != 0) {
      D_801D8F00[*kindp] = D_801D8EE8[*kindp];
      D_80148658 = D_80148658 + 1;
    }
  } else if (D_80145AA8 != 0) {
    func_8014ECAC(2);
    D_80148651 = D_80148651 + 1;
  }
}
