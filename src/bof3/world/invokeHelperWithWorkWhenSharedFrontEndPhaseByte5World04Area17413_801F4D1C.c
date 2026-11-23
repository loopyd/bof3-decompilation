#include "bof3/bof3.h"

/* Front-end phase byte of the shared main executable, outside this payload. The
 * neighbouring handler 0x801F3D40 of this overlay tests the same byte at
 * 0x80143BB0 against the same value 5, so the byte keeps the address-anchored
 * name this target has no map row for.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Scratchpad work-object pointer cell, outside this payload. The original
 * materialises the cell inside the guarded arm, so the record pointer is loaded
 * once and only when the guard holds.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Helper of the shared main executable at 0x8014E4BC. The bytes prove only that
 * this body calls it once with the work record as its single argument; its role
 * is not established, so the callee keeps the address-anchored name.
 */
void func_8014E4BC(void* work);

/* @source 0x801F4D1C
 * @behavior State handler of this overlay, reached only through the code
 *           pointer word 0x801F6654, entry 17 of the dispatch table this
 *           overlay places at 0x801F6610 inside its textbin blob, so no jal in
 *           the payload names it. When the shared front-end phase byte at
 *           0x80143BB0 reads 5 it passes the work record the scratchpad pointer
 *           cell 0x1F800044 holds to the main-executable helper 0x8014E4BC as
 *           its only argument; any other value leaves the record untouched.
 *           There is one test, no branch other than that guard and no store of
 *           its own, so the call is the whole effect; the 0x18-byte frame only
 *           keeps $ra at 0x10 across it and the delay slot stays a nop. Takes no
 *           arguments and returns nothing. The local handler 0x801F3D40 uses
 *           the same guard on the same shared byte and the exact sibling
 *           wrappers invokeHelperWithWorkWorld04Area19813_801F2FD4 and
 *           invokeHelperWithWorkWorld02Area08513_801F3BA4 pass the same cell to
 *           the same callee without the guard, which is where the statement
 *           order and operand forms come from.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWithWorkWhenSharedFrontEndPhaseByte5World04Area17413_801F4D1C(
    void) {
  if (D_80143BB0 == 5) {
    func_8014E4BC(D_1F800044);
  }
}
