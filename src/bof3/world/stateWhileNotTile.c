#include "bof3/bof3.h"

/* Shared main-RAM phase byte 0x80143BB0 that the neighbouring work-byte-1
 * handler 0x801F2D00 latches to 2 just before it moves the work record's
 * dispatch byte at 0x01 to 2; this handler advances only while that byte is not
 * yet 2. */
extern u8 D_80143BB0;

/* Shared main-RAM secondary sub-state byte of the front-end state machine. */
extern u8 D_80146875;

void renderFullscreenColorTile(void);

/* @source 0x801F2DBC
 * @behavior Entry 2 of the four-word work-byte-1 handler table at 0x801F3C54,
 * the entry the neighbouring dispatchStateByte1Table3c54World04Area19113_801F2C80
 * dispatcher selects with byte 0x01 of the scratch work record published at the
 * scratchpad pointer cell 0x1F800044. It reads the shared main-RAM phase byte
 * D_80143BB0 with `lbu` (`lui $v1,0x8014` / `lbu $v1,0x3BB0($v1)`) and compares
 * it against 2 (`addiu $v0,$zero,0x2` / `beq $v1,$v0`); only while that byte is
 * not 2 does it load the address of the shared front-end sub-state byte
 * D_80146875 (`lui $v1,0x8014` / `addiu $v1,$v1,0x6875`) and add one to that
 * byte (`lbu $v0,0x0($v1)` / `addiu $v0,$v0,0x1` / `sb $v0,0x0($v1)`), then
 * re-read the pointer cell and store 3 into the record's dispatch byte at 0x01,
 * so the next frame runs entry 3 of that table. Both arms rejoin at the
 * unconditional call to renderFullscreenColorTile,
 * made with no arguments and with its result discarded, which redraws the
 * fullscreen tile from the record's colour bytes at 0x5D, 0x5E and 0x5F; its
 * jal delay slot stays a nop. The 0x18-byte frame keeps only $ra at 0x10 across
 * that call, and $ra is stored in the delay slot of the shared-byte compare, so
 * the frame carries no other state. No word in this payload encodes a jal to
 * 0x801F2DBC, so the handler table is its only reference; it takes no arguments
 * and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateWhileNotTile(
    void) {
  u32 slot_offset;
  u8 *substate;

  if (D_80143BB0 != 2) {
    substate = &D_80146875;
    *substate += 1;
    slot_offset = 0x44u;
    PSX_REF(u8 *, SPAD_BASE + slot_offset)[1] = 3;
  }
  renderFullscreenColorTile();
}
