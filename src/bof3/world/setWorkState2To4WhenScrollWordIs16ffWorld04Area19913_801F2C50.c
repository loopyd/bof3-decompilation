#include "bof3/bof3.h"

/* Shared scroll word of the main executable, derived from the view-origin
 * mirror at 0x80149308 and packed as a position whose low byte stays 0xFF.
 * The bodies around it store and test values of the 0x7FFF minus a shifted
 * coordinate form through this word. */
extern u32 D_80149310;

/* @source 0x801F2C50
 * @behavior Overlay step hook: takes no arguments and returns no value. It
 *           reads the shared scroll word at 0x80149310 once with `lw` and
 *           compares it against the single packed position 0x16FF. Only on
 *           equality does it load the overlay work-record pointer published at
 *           the scratchpad cell 0x1F800044 and store 4 into that record's
 *           unsigned byte at offset 2, the step the overlay dispatcher indexes
 *           its handler run with. It allocates no frame, calls nothing, keeps
 *           nothing across a call, reads no other state and writes none other,
 *           and its only exit is the closing return through $ra; the 4 is
 *           materialised in $v0 inside the branch delay slot and the slot after
 *           the work-record pointer load stays a nop.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkState2To4WhenScrollWordIs16ffWorld04Area19913_801F2C50(void) {
  if (D_80149310 == 0x16FFu) {
    SPAD_PTR_SLOT(u8, 0x44)[2] = 4;
  }
}
