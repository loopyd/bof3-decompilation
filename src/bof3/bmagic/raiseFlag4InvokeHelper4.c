#include "bof3/bof3.h"

/* Shared battle global halfword at 0x801462E8. It sits outside this overlay's
 * image, in the main executable this target runs against, so it keeps the
 * address-anchored spelling that the generated undefined-syms map binds. The
 * neighbouring battle lifts name the same word; nothing in this payload
 * establishes a field role beyond the flag bit, so the name stays anchored. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* Shared main-executable helper at 0x801E5988, outside this overlay's image.
 * The exact sibling lifts in this tree call it with no arguments and keep no
 * result, so its role stays address-anchored and the generated undefined-funcs
 * map supplies the binding; no target-map row is added. */
extern void func_801E5988(void); /* @source 0x801E5988 */

/* @source 0x801EF03C
 * @behavior Entry stage 3 of this overlay: the frame dispatcher func_801EEC38
 *           copies the four words of the code pointer run at 0x801EEC04 onto
 *           its own frame and calls the one selected by work-object byte 0x1
 *           scaled by four, so the word at 0x801EEC10 names this body and no
 *           jal in the payload reaches it. It raises bit 0x4 of the shared
 *           battle global halfword at 0x801462E8 with an unsigned halfword
 *           read-modify-write, then invokes the shared helper at 0x801E5988
 *           with no arguments. The original materializes the halfword address
 *           once in a register because both the load and the store use it, so
 *           the access is written through a local pointer; the halfword store
 *           fills the call delay slot and the 0x18-byte frame only keeps $ra
 *           across the call. Takes no arguments, returns nothing and reads or
 *           writes no other state.
 *           Shape: this span is byte-identical across all 52 bytes to the
 *           exact lift raiseFlag4AndInvokeHelperBmagicMagic00800_801EEFDC at
 *           0x801EEFDC (one payload comparison of the two spans), whose
 *           statement order, operand forms and declaration style are copied
 *           here; the literal, the cell address and the callee are this
 *           span's own words.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4InvokeHelper(void) {
  u16 *flags = (u16 *)&D_801462E8;

  *flags |= 4;
  func_801E5988();
}
