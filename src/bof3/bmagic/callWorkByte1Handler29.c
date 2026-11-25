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

/* The four-word step run at the head of this target's leading table segment:
 * the step handlers 0x801EEC90, 0x801EED2C, 0x801EEE40 and 0x801EEFB8, the
 * four steps this overlay's work byte 0x01 selects between. The segment itself
 * holds eight pointers starting at payload offset 0x04; only this leading
 * four-word run is an object here. The original copies the whole run into its
 * frame as one record and dispatches through the copy, so it is one object
 * here too.
 * @source 0x801EEC04 @kind table
 */
extern HandlerTable D_801EEC04;

/* @source 0x801EEC24
 * @behavior Per-frame step dispatcher of this overlay, its first code span; no
 *           jal and no table word inside this payload names it, so the overlay
 *           loader is its only caller. It reads the scratchpad work-object
 *           pointer cell at 0x1F800044 once and takes that object's byte 0x01,
 *           the step selector, as an unchecked index: it copies the four-word
 *           step run at 0x801EEC04 (0x801EEC90, 0x801EED2C, 0x801EEE40,
 *           0x801EEFB8) into the 0x10($sp) record of its 0x28-byte frame and
 *           calls the entry at the scaled selector with no arguments. The
 *           selector is not bounded, and nothing of the selected step's result
 *           is returned here: the epilogue reloads only $ra. The run members
 *           move that selector themselves: the handler at 0x801EEC90 advances
 *           it, the handler at 0x801EED2C advances it once the countdown byte
 *           9 completes, the handler at 0x801EEE40 steps it back or forward,
 *           and the run's last entry at 0x801EEFB8 sets bit 0x4 of the shared
 *           flag word D_801462E8 as the closing action.
 *
 *           Shape and naming evidence: this 0x6C-byte span is byte-identical
 *           to the 0x801EEC14 span of emi/bmagic/magic115/03 and
 *           emi/bmagic/magic116/03, which a payload cmp proved for both before
 *           this body was written; the two spans carry the same table and
 *           pointer cells, so no word differs. The body is transcribed from
 *           dispatchWorkByte1HandlerBmagicMagic11603_801EEC14, which measured
 *           exact, and this overlay supplies the same table cell, pointer cell
 *           and unbounded selector from its own bytes. The clone adapter was
 *           declined: it stopped on the donor's function-pointer field with
 *           `unresolved external 'void'` before emitting a body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkByte1Handler29(void) {
  HandlerTable table;
  u8 *work;

  work = g_battle_work;
  table = D_801EEC04;
  table.handlers[work[1]]();
}
