#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, spelled the same way by
 * this overlay's exact lifts. The cell stays a named pointer rather than a
 * folded constant base: the original re-materialises its address with its own
 * lui 0x1F80 / lw 0x44 pair at the reload that follows the publish and the
 * restore stores, and a byte store through a loaded pointer may alias the cell,
 * which is what forces that reload. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer at 0x801EC2E0, outside this overlay image
 * and already bound by this target's generated undefined_syms_auto.txt. Word
 * 0x74 of that record is the saved work-object pointer: the bmagic family
 * writes the published work object into exactly that word, and this body takes
 * it back out and republishes it through the scratchpad cell. */
extern u8 *D_801EC2E0; /* @source 0x801EC2E0 @kind unknown */

/* Shared readiness helper of the main executable at 0x8014DAEC, outside this
 * overlay image and bound by this target's generated undefined_funcs_auto.txt.
 * The bmagic lifts declare the same zero-extended byte prototype; this body
 * calls it for its effect only and discards the returned byte. */
u8 func_8014DAEC(void);

/* @source 0x801EEE50
 * @behavior Republishes the shared battle record's saved work-object pointer
 *           through the scratchpad work-object pointer cell at 0x1F800044,
 *           invokes the shared readiness helper func_8014DAEC, restores the
 *           cell to the pointer it held, and then clears work byte 0x09 and
 *           work byte 0x0A. It reads the cell once into a callee-saved register
 *           before the publish, reads the record pointer 0x801EC2E0 and takes
 *           that record's word 0x74 as the value to publish, so the frame keeps
 *           the previous cell value at 0x10 and $ra at 0x14 across the single
 *           call. After the restore store the previous pointer is written back
 *           too: byte 0x09 is cleared through that held pointer and byte 0x0A
 *           through a fresh read of the cell, so the two clears keep separate
 *           base registers. Byte 0x09 is the stage index the per-frame
 *           dispatcher func_801EF620 advances through the three-entry stage
 *           table at 0x801EFADC and byte 0x0A is the per-stage frame countdown
 *           it decrements, both cleared here to restart the sequence; the roles
 *           of the publish and the readiness call stay with the bmagic family.
 *           The span is entry 1 of the four-word code pointer table at
 *           0x801EEC10 (0x801EED98, 0x801EEE50, 0x801EEEB0, 0x801EEFA4), which
 *           the handler at 0x801EED2C copies into its own frame and indexes
 *           with the scratchpad work object byte 0x02, so no jal in the image
 *           names it. Takes no arguments, returns nothing through the jr $ra at
 *           0x801EEEA8 inside the 0x18-byte frame, and reads or writes no other
 *           state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishSavedWorkPointerInvokeReadinessHelperThenClearWorkStageIndexAndCountdownBmagicMagic05703_801EEE50(void) {
  u8 *saved;

  saved = g_battle_work;
  g_battle_work = *(u8 **)(D_801EC2E0 + 0x74);
  func_8014DAEC();
  g_battle_work = saved;
  saved[9] = 0;
  g_battle_work[0xA] = 0;
}
