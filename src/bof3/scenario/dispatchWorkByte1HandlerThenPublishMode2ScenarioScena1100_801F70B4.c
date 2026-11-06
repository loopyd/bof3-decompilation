#include "bof3/bof3.h"

/* Overlay handler run at 0x801FAEC0, placed by the target's `textbin` row of the
 * same name and referenced here only. Its first three words are the following
 * three payload functions 0x801F7104, 0x801F7178 and 0x801F7200; twenty bytes of
 * byte data follow them, and the rest of the row (from 0x801FAEE0) is further
 * in-image code pointers. This dispatcher indexes the row from its base, so the
 * index is not bounded by the three leading code words. */
extern void (*D_801FAEC0[])(void);

/* Shared main-RAM mode byte, bound by config/targets/shared/symbols.txt. */
extern u8 D_80149333;

/* @source 0x801F70B4
 * @behavior Handler dispatch over work-record byte 1: reads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801FAEC0 with no arguments,
 * discarding any result. Once the handler returns it publishes mode 2 in the
 * shared mode byte D_80149333 (0x80149333), and that 2 is what $v0 still holds at
 * the return, so a caller sees 2 as the result. The original does not bound the
 * index before the scaled table load. The pointer-cell load precedes frame
 * creation, the 0x18-byte frame only keeps $ra at 0x10 across the indirect call,
 * and the slot after the record byte and the slot after the table load stay nops.
 * This is the payload's first code word at file offset 0x4B4 and no in-image jal
 * or pointer word names it, so the overlay loader that steps the leading
 * 0x801F73E8 pointer run enters it from outside this payload. The 80 bytes are
 * instruction-for-instruction identical to the exact dispatcher
 * func_8019A8B4 (0x8019A8B4) of emi/etc/game/00, which differs only in the
 * handler-run address it references (0x801C80E0 there); the address anchor keeps
 * the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerThenPublishMode2ScenarioScena1100_801F70B4(void) {
  D_801FAEC0[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
  D_80149333 = 2;
}
