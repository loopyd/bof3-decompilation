#include "bof3/bof3.h"

/* State-2 handler this step runs: it writes 3 to work state byte 0x02 when the
 * shared mode byte 0x801454F2 reads 2 and then draws halfword 0x2E. */
extern void
setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld03Area15113_801F3528(void); /* @source 0x801F3528 */

/* @source 0x801F34CC
 * @behavior Advances the work halfword at offset 0x2E of the record published
 *           at the scratchpad pointer cell 0x1F800044 by 0x10 and stores the
 *           truncated halfword. When the stored signed value is at least 0x10
 *           it increments work state byte 0x02. It then runs the state-2
 *           handler at 0x801F3528, which writes 3 to state byte 0x02 when the
 *           shared mode byte reads 2 and draws halfword 0x2E. The overlay
 *           reaches this step as entry 1 of the state-byte-2 handler table
 *           based at 0x801F5448 and as entry 3 of the state-byte-1 table based
 *           at 0x801F5440. Takes no arguments, uses no result, and its
 *           0x18-byte frame holds the return address only.
 *           The 0x5C bytes are equal to the exact
 *           advanceHalfword2eBy10AndAdvanceState2At10ThenDrawHalfword2eWorld03Area12114_801F3574
 *           span of emi/world03/area121/14 at its payload offset 0x974 and to
 *           the advanceHalfword2eBy10AndAdvanceState2At10ThenDrawHalfword2eWorld00Area03313_801F3490
 *           span of emi/world00/area033/13 at payload offset 0x890, apart from
 *           the `jal` word at offset 0x44, which carries each overlay's own
 *           state-2 handler (0x801F3528 here, 0x801F35D0 and 0x801F34EC there,
 *           each differing by exactly the offset delta between the two spans).
 *           Those cmps supplied this body's statement order, operand forms and
 *           name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword2eBy10AndAdvanceState2At10ThenDrawHalfword2eWorld03Area15113_801F34CC(
    void) {
  u8* cursor;
  s16* half;

  cursor = SPAD_PTR_SLOT(u8, 0x44);
  half = (s16*)(cursor + 0x2E);
  *half = (s16)(*half + 0x10);
  if (*half >= 0x10) {
    cursor[2]++;
  }
  setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld03Area15113_801F3528();
}
