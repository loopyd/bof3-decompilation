#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays,
 * bound at 0x1F800044 by this target's map. This body reads the cell once, in
 * its first statement, and keeps the pointer in a register across the handler
 * record copy, so the cell has no later access here. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* One handler step of this overlay's work object. */
typedef struct {
  void (*handlers[4])(void);
} HandlerTable;

/* The four-word step run of this target's leading table segment: the step
 * handlers 0x801EEC80, 0x801EEE7C, 0x801EEEC0 and 0x801EF03C, the four steps
 * this overlay's work byte 0x01 selects between. The original copies the whole
 * run into its frame as one record and dispatches through the copy, so it is
 * one object here too.
 * @source 0x801EEC04 @kind table
 */
extern HandlerTable D_801EEC04;

/* @source 0x801EEC14
 * @behavior Per-frame step dispatcher of this overlay, the first code span
 *           behind the four-word step run of the leading table segment at
 *           payload offset 0x14; no jal immediate and no pointer word anywhere
 *           in this payload names it, so the overlay loader is its only caller.
 *           It reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           and takes that object's byte 0x01, the overlay's step selector, as
 *           an unchecked index: it copies the whole step run at 0x801EEC04
 *           (0x801EEC80, 0x801EEE7C, 0x801EEEC0, 0x801EF03C) into the
 *           0x10($sp) record of its 0x28-byte frame and calls the entry at the
 *           scaled selector with no arguments. The selector is not bounded, and
 *           nothing of the selected step's result is returned here: the
 *           epilogue reloads only $ra. This 0x6C-byte span is byte-identical to
 *           the same payload offset of emi/bmagic/magic115/03 and
 *           emi/bmagic/magic116/03, which a payload comparison proved for both
 *           before this seed was written, so the statement order, operand forms
 *           and record copy come from those two lifts; the four run members
 *           written here are this payload's own words. Step 1 of that run,
 *           stateBarrierSevenSet,
 *           was lifted from this payload and moves the selector from 1 to 2,
 *           which hands the next dispatch to word 2 at 0x801EEEC0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkByte1Handler26(void) {
  HandlerTable table;
  u8 *work;

  work = g_battle_work;
  table = D_801EEC04;
  table.handlers[work[1]]();
}
