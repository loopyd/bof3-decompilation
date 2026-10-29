#include "bof3/bof3.h"

extern u8 D_801F68A0;
extern u8 D_801F689C;

/* @source 0x801F2F30
 * @behavior Overlay work-state entry: resets the scratchpad work object
 * published through the pointer cell at 0x1F800044 by clearing its byte 9 and
 * writing 1 into its work-state byte 1, then publishes 0x0F to the payload
 * shade byte at 0x801F68A0 and clears its selector byte at 0x801F689C. The
 * cell is indexed as scratchpad pointer cell 0x11 (address 0x1F800044) and
 * evaluated twice, so the aliasable byte-9 store forces the second load to be
 * re-issued rather than reused; the object is otherwise untouched and the
 * function takes no argument, makes no call and returns nothing.
 *
 * Evidence: payload asm at 0x801F2F30 -- lui/lw of cell 0x1F800044, sb $zero,
 * 0x9($v0); lui/lw of the same cell again, li 1, sb $v0,0x1($v1); li 0xF,
 * sb $v0,%lo(D_801F68A0); sb $zero,%lo(D_801F689C); jr $ra. 0x801F2F30 is
 * entry 0 of the payload handler pointer table at 0x801F4DF4, whose entry 1 is
 * func_801F2F6C: that handler steps work byte 9 up to 0x10, loads the byte at
 * 0x801F4DF0 indexed by 0x801F689C into 0x801F68A0 and copies 0x801F68A0 into
 * the red component of a POLY_G4 primitive, which is the shade/selector pair
 * written here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte9SelectWorkState1AndShade0FWorld04Area18813_801F2F30(void) {
  u8** slots;

  slots = SPAD_PTR_TABLE(u8);
  slots[0x11][9] = 0;
  slots[0x11][1] = 1;
  D_801F68A0 = 0x0F;
  D_801F689C = 0;
}
