#include "bof3/bof3.h"

/* This overlay's handler run at 0x801D1B1C: fourteen in-image code pointers from
 * 0x801D0C70 (entry 0) through 0x801D11A4 (entry 13) at 0x801D1B50, the last word
 * before the payload's zero padding. The target's Splat `textbin` row binds the run
 * base as func_801D1B1C and the generated `undefined_syms_auto` rows name the three
 * interior words the step dispatchers index, so this entry reaches the run through
 * the row's own base name. */
extern void (*func_801D1B1C[])(void);

/* The draw helper at 0x801D1218 is a separate payload span, still on its `asm` row:
 * it takes the record this call site passes at work +0x34, the signed halfword it
 * loads with `lh` at work +0xC (the radius it scales its ring of 0x1C-byte sprite
 * packets by) and the flip-flag word whose three bits it turns into 0x80 texture
 * offsets before queueing each packet under OT slot 7. */
void func_801D1218(u8* record, s16 radius, s32 flip_flags);

/* @source 0x801D0C04
 * @behavior The overlay entry step. It loads the work-object pointer published at
 * the scratchpad pointer cell 0x1F800044 before it creates its frame, takes that
 * object's unsigned byte at offset +0x1 and scales it by four to call that entry of
 * the fourteen-word handler run at 0x801D1B1C with no arguments, discarding any
 * result; the original does not bound the index before the scaled table load. It
 * then reloads the same pointer cell into a second local, because the indirect call
 * may republish that cell, and gates on the same work byte: while that byte is
 * non-zero it hands func_801D1218 the work object's record at +0x34, the signed
 * halfword at +0xC and the literal 7. It takes no arguments and returns nothing;
 * the 0x18-byte frame only keeps $ra at 0x10 across the two calls, and the record
 * pointer's address computation fills the guard branch's delay slot.
 *
 * Evidence: no `jal` and no word of this payload's only pointer run (the fourteen
 * words at 0x801D1B1C through 0x801D1B50) addresses 0x801D0C04, so the overlay
 * loader outside the payload enters it, exactly as it enters the step dispatchers.
 * The body is the shape of the exact sibling 0x801D0EF8 in this same target (same
 * dispatch of the same run, then a guarded draw through a reloaded pointer cell)
 * carrying this span's own table base, callee, argument forms and literal: the two
 * differ in the table base word, in the callee and its three arguments, and in the
 * guard's delay slot, which holds the record address here and stays nop there
 * because that sibling's first drawn operand is a halfword load.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerThenDrawWorkRecordWhenActiveScenarioSce15ef200_801D0C04(
    void) {
  u32 slot_offset;
  u8* work;
  u8* next;

  slot_offset = 0x44u;
  work = PSX_REF(u8*, SPAD_BASE + slot_offset);
  func_801D1B1C[work[1]]();
  next = PSX_REF(u8*, SPAD_BASE + slot_offset);
  if (next[1] != 0) {
    func_801D1218(next + 0x34, *(s16*)(next + 0xC), 7);
  }
}
