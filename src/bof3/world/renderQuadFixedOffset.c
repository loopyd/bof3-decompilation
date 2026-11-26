#include "bof3/bof3.h"

void func_801F31E8(s32* args, s32 offset);

/* Shared published work-record pointer cell of the scratchpad. The original
 * materialises the cell address once and keeps the pointer in a register, which
 * is why the load precedes the frame setup. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F31A0
 * @behavior Overlay step handler, entry 2 of the handler table based at
 *           0x801F4084 that this target's dispatcher
 *           dispatchWorkByte1Handler3 indexes with
 *           the work record's state byte at +1. Entry 0 at 0x801F30B4 seeds the
 *           record's words and entry 1 at 0x801F3108 rises the record's +0x14
 *           word and advances that state byte once +0x14 passes 0x07FFFFFF, so
 *           this entry is the state that follows that rise. It loads the work
 *           record published at the scratchpad cell 0x1F800044, copies the
 *           record's words at +0x34, +0x38 and +0x3C into a three-word local
 *           array, and calls the overlay draw routine at 0x801F31E8 with that
 *           array and the fixed word 0x08000000. That routine adds its second
 *           argument to the array's third word before projecting it through
 *           0x801AFF04, so this handler draws the record's quad at the fixed
 *           offset 0x08000000 and holds it there. It touches no other record
 *           word, advances no state byte and takes no arguments; its 0x28-byte
 *           frame holds the 12-byte array at 0x10 and $ra at 0x20.
 *           Name evidence: the only in-image reference to 0x801F31A0 is the
 *           third word of the three-entry pointer run based at 0x801F4084,
 *           which the exact dispatcher
 *           dispatchWorkByte1Handler3 indexes with
 *           the record's byte at +1; there is no jal caller, which is the
 *           dispatch shape rather than missing evidence. The quad-draw role
 *           comes from this body's own stores and the callee's own bytes.
 *           Shape evidence: 18-word scan of every extracted payload finds no
 *           other copy of these bytes, so no clone applies. The exact
 *           same-target sibling at 0x801F3108 calls 0x801F31E8 with the same
 *           0x28-byte frame, the same 12-byte array at 0x10 and the same unfilled
 *           load delay slots, and the exact same-target sibling at 0x801F2D18
 *           holds the same copy shape with the cell load ahead of the frame
 *           setup, which is the order this span's bytes show. Statement order,
 *           operand forms and declaration style follow those two; this body's
 *           own callee, constant and record offsets come from its original
 *           bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderQuadFixedOffset(void) {
  s32 args[3];
  u8* work;

  work = D_1F800044;
  args[0] = FIELD_REF(s32, work, 0x34);
  args[1] = FIELD_REF(s32, work, 0x38);
  args[2] = FIELD_REF(s32, work, 0x3C);
  func_801F31E8(args, 0x8000000);
}
