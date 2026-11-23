#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell published by the front end. The original
 * re-reads it with a fresh `lui`/`lw` pair before every store, so this body
 * addresses it by name at each access instead of folding the address into one
 * base register.
 * @source 0x1F800044 @kind unknown
 */
extern u8* scratchpadWorkRecord;

void func_8014DD3C(u16 entry);

/* @source 0x801F2FD0
 * @behavior Entry 0 of the five-entry work-byte-1 handler run based at
 *           0x801F542C, which the overlay update at 0x801F2E5C indexes with
 *           the work record's byte 0x01. The body's address appears once in
 *           the payload, at that table's first word, and no `jal` names it, so
 *           it takes no arguments and answers no value. It stores 0x80 in work
 *           byte 0x24, applies main lookup entry 0x1D1 through the resident
 *           0x8014DD3C helper, stores 5 in work byte 0x29, clears work byte
 *           0x2A and the colour triplet 0x5D/0x5E/0x5F, and finally arms the
 *           state byte at 0x01 with 1 so the next dispatch takes entry 1 of the
 *           same run at 0x801F305C. The scratchpad pointer cell 0x1F800044 is
 *           re-read before every store, matching the original's per-store
 *           reloads. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The 0x8C bytes at payload offset 0x3D0 are byte-identical to the func_801F2F24
 * span at payload offset 0x324 in emi/world04/area152/13 in one cmp of each
 * payload span at its own offset; that twin is still on its `asm` row, so the
 * clone lever had no source to clone and was declined. Each already-exact
 * family member differs from this span in exactly one word - the
 * `addiu $a0, $zero, <lookup>` immediate at offset 0x10: World02Area08713 at
 * payload offset 0x494 (0x156), World02Area08813 at 0xFC4 (0x157),
 * World03Area12114 at 0x478 (0x158) and World01Area06513 at 0x498 (0x53) -
 * and their source supplied the statement order, operand forms and declaration
 * style. This body carries no payload-relative word: every absolute address it
 * names is a shared cell outside the overlay, so the only literal written here
 * is this span's own 0x1D1 lookup entry.
 */
void prepareWorkRecordForLookup1D1AndArmState1World03Area15113_801F2FD0(
    void) {
  scratchpadWorkRecord[0x24] = 0x80;
  func_8014DD3C(0x1D1);
  scratchpadWorkRecord[0x29] = 5;
  scratchpadWorkRecord[0x2A] = 0;
  scratchpadWorkRecord[0x5D] = 0;
  scratchpadWorkRecord[0x5E] = 0;
  scratchpadWorkRecord[0x5F] = 0;
  scratchpadWorkRecord[0x01] = 1;
}
