#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell, the target map's own name for
 * 0x1F800044. The original re-reads it with a fresh `lui`/`lw` pair before
 * every store, so this body addresses the cell by name at each access instead
 * of folding its address into one base register.
 * @source 0x1F800044 @kind unknown
 */
extern u8* area15213WorkCursor_1F800044;

/* Shared main-executable lookup helper: this target's generated
 * undefined_funcs_auto row binds the address-anchored name, and the byte-equal
 * lift in emi/world03/area151/13 declares this narrow arity. */
void func_8014DD3C(u16 entry);

/* @source 0x801F2F24
 * @behavior Entry 0 of the five-entry work-byte-1 handler run based at
 *           0x801F4D58, which the overlay update at 0x801F2DB0 indexes with
 *           the work record's byte 0x01; the run's remaining words read
 *           0x801F2FB0, 0x801F30E8, 0x801F3164 and 0x801F3224 before the next
 *           table base at 0x801F4D6C. This body's address appears once in the
 *           payload, at that run's first word, and no `jal` names it, so it
 *           takes no arguments and answers no value.
 *           It stores 0x80 in work byte 0x24, applies main lookup entry 0x1D1
 *           through the resident 0x8014DD3C helper, stores 5 in work byte 0x29,
 *           clears work byte 0x2A and the colour triplet 0x5D/0x5E/0x5F, and
 *           finally arms the state byte at 0x01 with 1 so the next dispatch
 *           takes entry 1 of the same run at 0x801F2FB0. The scratchpad pointer
 *           cell 0x1F800044 is re-read before every store, matching the
 *           original's per-store reloads. Its 0x18-byte frame holds the return
 *           address only.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Its 0x8C bytes at payload offset 0x324 measured byte-identical to the 0x8C
 * bytes at payload offset 0x3D0 of emi/world03/area151/13 in one cmp of each
 * payload span at its own offset. That span's lift,
 * prepareWorkRecordForLookup1D1AndArmState1World03Area15113_801F2FD0,
 * measured exact and supplied this body's statement order, operand forms and
 * declaration style; every literal and binding below was read from this
 * target's own payload and map.
 */
void prepareWorkRecordForLookup1D1AndArmState1World04Area15213_801F2F24(
    void) {
  area15213WorkCursor_1F800044[0x24] = 0x80;
  func_8014DD3C(0x1D1);
  area15213WorkCursor_1F800044[0x29] = 5;
  area15213WorkCursor_1F800044[0x2A] = 0;
  area15213WorkCursor_1F800044[0x5D] = 0;
  area15213WorkCursor_1F800044[0x5E] = 0;
  area15213WorkCursor_1F800044[0x5F] = 0;
  area15213WorkCursor_1F800044[0x01] = 1;
}
