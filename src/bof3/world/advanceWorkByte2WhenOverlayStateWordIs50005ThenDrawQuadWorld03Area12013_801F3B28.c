#include "bof3/bof3.h"

/* Shared overlay state word. This handler reads the whole 32-bit word and
 * compares it against a packed pair of halfwords, so the word load belongs to
 * this target's own use; the leading halfword 0x80143B90 is the shared state
 * cell other scenes write single small values through.
 * @source 0x80143B90 @kind data
 */
extern u32 D_80143B90;

/* Quad drawer at 0x801F3F5C, still unlifted assembly inside this target, so
 * only its prototype is declared. Its head adds the third argument to the work
 * record byte at 0x0A and wraps it modulo 0x0C, and its body builds a textured
 * four-point polygon through SetPolyFT4, SetShadeTex and RotTransPers4. */
void func_801F3F5C(u32 arg0, u32 arg1, u32 arg2, u32 arg3);

/* @source 0x801F3B28
 * @behavior Reads the shared overlay state word at 0x80143B90 with one 32-bit
 *           load and compares it against the packed halfword pair 0x00050005.
 *           While that word matches, it advances byte 0x02 of the scratchpad
 *           work record published through pointer cell 0x1F800044 by one,
 *           reading and storing the byte through the same pointer value. Byte
 *           0x02 is the record byte this overlay's own work-byte-2 dispatcher
 *           table at 0x801F43E8 indexes, so the increment moves the next frame
 *           to the following handler. It then calls the quad drawer
 *           func_801F3F5C with the fixed arguments (0x80, 0x40, 1, 0). Takes no
 *           arguments, returns nothing, and its 0x18-byte frame holds only the
 *           saved return address at 0x10. The roles of the state word beyond
 *           the match, of the record byte it advances and of the four callee
 *           arguments stay described by address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte2WhenOverlayStateWordIs50005ThenDrawQuadWorld03Area12013_801F3B28(
    void) {
  if (D_80143B90 == 0x00050005u) {
    SPAD_PTR_SLOT(u8, 0x44)[2]++;
  }
  func_801F3F5C(0x80, 0x40, 1, 0);
}
