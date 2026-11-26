#include "bof3/bof3.h"

/* The scratchpad pointer cell that publishes the current work record. The
 * original re-materialises this cell for each access rather than keeping the
 * address in a saved register, so both reads here go through the named cell and
 * each emits its own `lui`/`lw` pair. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Handler run of this overlay: the reviewed Splat `textbin` row based at
 * 0x801F3554, whose entry 0 is 0x801F2C60 (clear work byte 9, advance byte 2)
 * and whose entry 1 is 0x801F2C90 (advance byte 9 by two). Both words are code
 * pointers, and this overlay entry indexes the run with the work record's own
 * byte 2. */
extern void (*tableStepHandlers[])(void); /* @source 0x801F3554 @kind data */

/* Step consumer of this overlay, the span that follows this entry. It takes the
 * work record's step byte in $a0 and moves it into a saved register, so the
 * parameter is a byte value. */
extern void stateConsumeStepByte(u8 step); /* @source 0x801F2CB0 */

/* @source 0x801F2C04
 * @behavior Overlay entry of the world03 area142 step handler. The leading two
 *           instructions read the work record pointer published at the
 *           scratchpad pointer cell 0x1F800044 (`lui v0,0x1F80`, `lw
 *           v0,0x44(v0)`) before the 0x18-byte frame is reserved, then the body
 *           takes that record's unsigned byte 2, scales it by four and calls
 *           the selected entry of the handler run based at 0x801F3554 through
 *           `jalr v0` with a `nop` delay slot. Byte 2 is the state byte the
 *           run's own handlers move - 0x801F2C60 advances it by one while it
 *           clears byte 9, and 0x801F2C90 leaves it alone while it advances
 *           byte 9 by two - so it selects the handler for the record's current
 *           step. Once that handler returns, the body re-reads the same
 *           scratchpad cell and passes the record's unsigned byte 9 to the
 *           overlay's step consumer at 0x801F2CB0, which the run's handlers
 *           advance and this entry reads afterwards. The 0x18-byte frame only
 *           keeps $ra at 0x10 across the two calls, the slot after the index
 *           byte and the slots after the table load and the consumer call stay
 *           nops, and no other state is read or written. No call or pointer
 *           word in the payload names this function, so it is the overlay's
 *           loader entry rather than an in-payload callee. Takes no arguments
 *           and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void
callRunStepConsumer(void) {
  tableStepHandlers[D_1F800044[2]]();
  stateConsumeStepByte(D_1F800044[9]);
}
