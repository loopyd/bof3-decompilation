#include "bof3/bof3.h"

/* Scratchpad work-record cursor cell 0x1F800044, bound by this target's own map
 * row. Reading it through the named cell emits the original's lui %hi plus
 * offset lw at every store group instead of holding one folded base in a saved
 * register. */
extern u8* D_1F800044;

/* Shared resident main-lookup helper at 0x8014DD3C: copies the record whose
 * 16-bit key matches the entry id into the active work record and returns
 * nothing this caller uses. The overlay reaches it through the generated
 * func_8014DD3C row in undefined_funcs_auto.txt. */
void func_8014DD3C(u16 entry);

/* @source 0x801F3064
 * @behavior Entry 0 of the work-record state table at 0x801F4F48, which the
 *           update at 0x801F2EF0 indexes with the work-record byte at 0x01. It
 *           stores 0x80 in work byte 0x24, applies main lookup entry 0x1C7
 *           through the resident func_8014DD3C helper, stores 5 in work byte
 *           0x29, clears work bytes 0x2A and 0x5D..0x5F, and finally arms the
 *           state byte at 0x01 with 1 so the next dispatch runs the state-1
 *           handler at 0x801F30F0. The scratchpad pointer cell 0x1F800044 is
 *           re-read before every store group. Takes no arguments and returns
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void prepareWorkRecordArm(void) {
  D_1F800044[0x24] = 0x80;
  func_8014DD3C(0x1C7);
  D_1F800044[0x29] = 5;
  D_1F800044[0x2A] = 0;
  D_1F800044[0x5D] = 0;
  D_1F800044[0x5E] = 0;
  D_1F800044[0x5F] = 0;
  D_1F800044[0x01] = 1;
}
