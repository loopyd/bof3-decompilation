#include "bof3/bof3.h"

/* State-2 tail this step runs: it writes 3 to work state byte 0x02 when the
 * shared mode byte 0x801454F2 reads 2 and then draws halfword 0x2E. */
extern void
setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld00Area03313_801F34EC(void); /* @source 0x801F34EC */

/* @source 0x801F3490
 * @behavior Advances the work halfword at offset 0x2E of the record published
 *           at the scratchpad pointer cell 0x1F800044 by 0x10 and stores the
 *           truncated halfword. When the stored signed value is at least 0x10 it
 *           increments work state byte 0x02. It then runs the state-2 handler at
 *           0x801F34EC, which writes 3 to state byte 0x02 when the shared mode
 *           byte reads 2 and draws halfword 0x2E with 0x10. Takes no arguments,
 *           uses no result, and its 0x18-byte frame holds the return address
 *           only. All 0x5C bytes matched the already reviewed
 *           advanceHalfword2eBy10AndAdvanceState2At10ThenDrawHalfword2eWorld03Area12114_801F3574
 *           apart from the `jal` word at offset 0x44, which carries each
 *           overlay's own state-2 handler (0x801F34EC here, 0x801F35D0 there),
 *           so this body takes that sibling's statement order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword2eBy10AndAdvanceState2At10ThenDrawHalfword2eWorld00Area03313_801F3490(
    void) {
  u8* cursor;
  s16* half;

  cursor = SPAD_PTR_SLOT(u8, 0x44);
  half = (s16*)(cursor + 0x2E);
  *half = (s16)(*half + 0x10);
  if (*half >= 0x10) {
    cursor[2]++;
  }
  setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld00Area03313_801F34EC();
}
