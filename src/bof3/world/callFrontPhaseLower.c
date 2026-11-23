#include "bof3/bof3.h"

/* Front-end phase byte of the shared main executable, outside this payload. The
 * neighbouring handlers of this overlay's code run test the same byte against
 * the same value 5, so the byte keeps the address-anchored name the shared map
 * binds.
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

/* Helper of the shared main executable at 0x8014E4BC. The bytes prove only that
 * this body passes it the work record the scratchpad cell supplies; its role is
 * not established, so the callee keeps its address-anchored name.
 */
void func_8014E4BC(void* work);

/* @source 0x801F35A4
 * @behavior Handler entry 8 of the thirteen-word code-pointer run at 0x801F4268
 *           inside the reviewed textbin row D_801F3F10, so no jal in this
 *           payload names it. When the shared front-end phase byte at
 *           0x80143BB0 reads 5 it passes the work record held by the scratchpad
 *           pointer cell at byte offset 0x44 - cell 0x11 of the scratchpad
 *           pointer table - to the main-executable helper 0x8014E4BC as its
 *           only argument and then jumps to the shared epilogue; any other value
 *           takes the fall-through arm, which loads the global pointer cell
 *           0x80148210 once and lowers the halfword at its +0x0A offset by 2.
 *           Takes no arguments and returns nothing. The 0x18-byte frame keeps
 *           only $ra at +0x10, the store of $ra fills the delay slot of the
 *           guard's `bne`, the two load-delay slots of the cell arm stay nops,
 *           the call's delay slot stays a nop, and both arms leave through one
 *           shared epilogue.
 *
 *           The span of func_801F3CE4 at 0x801F3CE4 of emi/world04/area174/13
 *           compared equal to this one word for word except its own `j`
 *           immediate, which is 0x1D0 higher, exactly its span's distance from
 *           this one in four-byte words, so that one body supplies the statement
 *           order, the operand forms and the arm topology here. The guarded arm
 *           also matches the wrapper func_801F4D1C of the same overlay and the
 *           wrapper func_801F2FD4 of this target, which reach the same helper
 *           through the same scratchpad cell, and the fall-through arm matches
 *           the handler func_801F3D40 of that overlay and the handler
 *           func_801F3854 of this one, whose halfword form this body writes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callFrontPhaseLower(
    void) {
  if (D_80143BB0 == 5) {
    func_8014E4BC(SPAD_PTR_TABLE(u8)[0x11]);
  } else {
    D_80148210[5] -= 2;
  }
}
