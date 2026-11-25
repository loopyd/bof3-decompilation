#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F30D0
 * @behavior Queues the three shared UI elements at x = 0x52 with y = 0x32,
 *           0x33 and 0x34, every one at size 0, through the shared marker
 *           queue func_801A4BC0. Takes no arguments and returns nothing, and
 *           its 0x18-byte frame only keeps $ra across the three calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the three jal instructions at 0x801F30E0/0x801F30F0/
 * 0x801F3100 all target the shared marker queue func_801A4BC0. Each call sets
 * its arguments in the two words before that jal and finishes in the delay
 * slot, and those words are this function's own literals from the original
 * instruction words - a0 = 0x52 in all three (0x801F30D8/0x801F30E8/
 * 0x801F30F8), a1 = 0x32 then 0x33 then 0x34 (0x801F30DC/0x801F30EC/
 * 0x801F30FC) and a2 = 0 in all three delay slots. That is one x column
 * holding a three-cell run of consecutive y values at a single size, so the
 * name follows the block grammar of
 * queueSharedUiElementBlock40x27To28Size0World01Area04113_801F41F8 and
 * queueSharedUiElementBlock9x59To5BSize51World03Area13613_801F2E08.
 *
 * Shape notes from the original bytes: a payload cmp of this 0x48-byte span
 * against the exact sibling func_801F2FD0 at 0x801F2FD0 of
 * emi/world03/area136/13 finds the same body with nine differing words, all of
 * them the three argument literals of each call (a0 0x52 here against 0x9
 * there, a1 0x32/0x33/0x34 against 0x59/0x5A/0x5B, a2 0 against 0x10), so
 * statement order, operand forms and declaration style are taken from that
 * sibling and only this function's own cell and size literals are written from
 * these bytes. The 0x48-byte body has its own prologue, its three calls and its
 * own jr $ra at 0x801F3110, and the next subsegment begins at 0x801F3118, so
 * this is a whole function rather than data or a shared epilogue. The clone
 * lever does not apply: a search of every payload under out/binaries for these
 * 0x48 bytes finds only this copy. No jal immediate targets 0x801F30D0 and no
 * word anywhere in this payload equals 0x801F30D0, so no caller or handler run
 * names an extra role and the role stays anchored to the queued cell
 * arguments.
 */
void renderQueueMarkerTriple(void) {
  func_801A4BC0(0x52, 0x32, 0);
  func_801A4BC0(0x52, 0x33, 0);
  func_801A4BC0(0x52, 0x34, 0);
}
