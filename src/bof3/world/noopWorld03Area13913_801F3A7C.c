#include "bof3/bof3.h"

/* @source 0x801F3A7C
 * @behavior Empty overlay function: it reads and writes no state and returns
 *           to its caller, but its prologue reserves a 0x10 stack frame and its
 *           epilogue releases it (addiu $sp,$sp,-0x10 / addiu $sp,$sp,0x10 /
 *           jr $ra / nop: 4 instructions, 16 bytes). No word of this payload
 *           references 0x801F3A7C, so no local dispatch table selects it; the
 *           overlay is entered from outside its payload and its prologue and
 *           return path alone prove the boundary. No argument is read and no
 *           value is returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The frame slot is a reconstruction of a declaration the original translation
 * unit carried but whose emitted code never references it: this body needs no
 * storage of its own, the prologue is a bare addiu $sp,-0x10 and the epilogue
 * a bare addiu $sp,0x10, and no instruction in the 16 original bytes touches a
 * $sp offset. Declaring one 16-byte local restores that reservation; the name
 * and type are a reconstruction choice constrained only by the slot size. Same
 * recorded shape as the frame slots in
 * seedThreeRotatingQuadRecordsWorld02Area10313_801F349C,
 * advanceWorkByte1WhenNoRecordActiveWorld02Area10814_801F2CC8 and
 * func_801F7670. Flagged for independent review; removing the declaration
 * leaves only the frame-size residual.
 */
void noopWorld03Area13913_801F3A7C(void) {
  u32 unusedFrameSlot[4];

  (void)unusedFrameSlot;
}
