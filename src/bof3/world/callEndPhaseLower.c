#include "bof3/bof3.h"

/* Front-end phase byte of the shared main executable, outside this payload. The
 * neighbouring handlers 0x801F3D40 and 0x801F4D1C of this overlay test the same
 * byte against the same value 5, so the byte keeps the address-anchored name
 * this target has no map row for.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Record published through the global pointer cell at 0x80148210, outside this
 * payload. The original loads the cell once with one `lw` and then reads and
 * writes its +0x0A halfword through that loaded pointer, so the cell is
 * declared at halfword width.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;

/* Scratchpad work-object pointer cell, outside this payload. The original
 * materialises the cell inside the guarded arm, so the record pointer is loaded
 * once and only when the guard holds.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Helper of the shared main executable at 0x8014E4BC. The bytes prove only that
 * this body passes it the work record the scratchpad cell supplies; its role is
 * not established, so the callee keeps its address-anchored name.
 */
void func_8014E4BC(void* work);

/* @source 0x801F3CE4
 * @behavior Handler entry 1 of the code-pointer run at 0x801F6610 inside the
 *           textbin blob of this overlay, so no jal in the payload names it.
 *           When the shared front-end phase byte at 0x80143BB0 reads 5 it passes
 *           the work record the scratchpad pointer cell 0x1F800044 holds to the
 *           main-executable helper 0x8014E4BC as its only argument and then
 *           jumps to the shared epilogue; any other value takes the fall-through
 *           arm, which loads the global pointer cell 0x80148210 once and lowers
 *           the halfword at its +0x0A offset by 2. Takes no arguments and
 *           returns nothing. The 0x18-byte frame keeps only $ra at +0x10, the
 *           store of $ra fills the delay slot of the guard's `bne`, the two
 *           load-delay slots of the cell arm stay nops, the call's delay slot
 *           stays a nop, and both arms leave through one shared epilogue.
 *
 *           The span of 0x801F35A4 in emi/world04/area198/13 compared equal to
 *           this one word for word except its own `j` immediate, which is 0x1D0
 *           lower there - exactly the 0x740 bytes between the two spans in
 *           four-byte words - so that body supplied the statement order, the
 *           operand forms and the arm topology here. The guarded arm also
 *           matches the wrapper 0x801F4D1C of this overlay, which reaches the
 *           same helper through the same scratchpad cell, and the fall-through
 *           arm matches the handler 0x801F3D40 of this overlay, whose halfword
 *           form this body writes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callEndPhaseLower(
    void) {
  if (D_80143BB0 == 5) {
    func_8014E4BC(D_1F800044);
  } else {
    D_80148210[5] -= 2;
  }
}
