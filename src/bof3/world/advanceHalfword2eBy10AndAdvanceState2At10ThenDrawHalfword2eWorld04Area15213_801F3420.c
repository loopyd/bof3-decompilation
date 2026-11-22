#include "bof3/bof3.h"

/* State-2 tail this step runs: it writes 3 to work state byte 0x02 when the
 * shared mode byte 0x801454F2 reads 2 and then draws halfword 0x2E. */
void setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld04Area15213_801F347C(void);

/* @source 0x801F3420
 * @behavior Entry 1 of the work-byte-2 handler run based at 0x801F4D74 in this
 *           overlay, whose words from 0x801F4D74 read 0x801F33D0, 0x801F3420
 *           and 0x801F347C, so this is the state the dispatcher 0x801F338C
 *           reaches while work record byte 0x02 reads 1; the same word also
 *           sits at 0x801F4D78, index 8 of the in-image code-pointer run from
 *           0x801F4D58 through 0x801F4D90. It advances the work halfword at
 *           offset 0x2E of the record published at the scratchpad pointer cell
 *           0x1F800044 by 0x10, stores the truncated halfword, and increments
 *           work state byte 0x02 once the stored signed value is at least 0x10.
 *           It then calls the overlay's state-2 step 0x801F347C, which writes 3
 *           to state byte 0x02 when the shared mode byte reads 2 and draws
 *           halfword 0x2E with 0x10. Takes no arguments, uses no result, and
 *           its 0x18-byte frame holds the return address only.
 *           In one cmp its 0x5C bytes were equal to the
 *           advanceHalfword2eBy10AndAdvanceState2At10ThenDrawHalfword2eWorld03Area12114_801F3574
 *           span of emi/world03/area121/14 at payload offset 0x974 apart from
 *           the `jal` word at offset 0x44, which carries each overlay's own
 *           state-2 step (0x801F347C here, 0x801F35D0 there), so that exact
 *           lift supplied this body's statement order, operand forms and name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword2eBy10AndAdvanceState2At10ThenDrawHalfword2eWorld04Area15213_801F3420(
    void) {
  u8* cursor;
  s16* half;

  cursor = SPAD_PTR_SLOT(u8, 0x44);
  half = (s16*)(cursor + 0x2E);
  *half = (s16)(*half + 0x10);
  if (*half >= 0x10) {
    cursor[2]++;
  }
  setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld04Area15213_801F347C();
}
