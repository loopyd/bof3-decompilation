#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F2CEC
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8); while that byte is nonzero it stores the
 *           constant 0x44 into the shared status halfword at 0x801490A8 (`sh`,
 *           with the 0x44 materialised in the branch delay slot) and then 6 into
 *           the shared mode byte at 0x801D4285 (`addiu v0,zero,6`, `sb`); when
 *           the gate byte is zero it stores 0xFFFF into D_801490A8 instead
 *           (`ori` off $zero) and touches nothing else. Both arms join at one
 *           `jr $ra`. Takes no arguments and returns nothing; leaf with no
 *           frame, and each store site materialises its own `$at` page.
 *         Shape: a word-by-word comparison of this 68-byte span against this
 *           target's 0x801F2CA8 body (one payload read of both spans, not a
 *           lift status) found exactly one differing word, the internal `j`
 *           immediate, which moves by 0x11 in the encoded target field - the
 *           0x44-byte span delta shifted for the encoded branch target - so
 *           every relative branch, stored literal, operand form, cell reference
 *           and offset spelling is equal and the two spans are one body. That
 *           body supplied the statement order, the arm order, the branch
 *           polarity and the declaration style; this span's own bytes supplied
 *           the 0x44/6/0xFFFF literals and the three cell addresses, all of
 *           which it shares.
 *         Boundary and role: the only reference to this address in the payload
 *           is the address-taken word at 0x801F333C, inside the pointer run at
 *           0x801F3324..0x801F333C, with no `jal` immediate anywhere, so the
 *           role stays address-taken, the run's neighbours carry no prototype,
 *           and the empty parameter list is structural. The body ends with its
 *           own `jr $ra` and the 0x44-byte span ends exactly at the next
 *           subsegment start 0x801F2D30.
 *         Levers: data declined - the span holds `beqz`/`j`/`jr $ra` control
 *           flow and no table shape; carve declined - the span is a whole leaf
 *           body with its own return and a start that the pointer run names
 *           exactly; clone declined - one scan of the 398 payloads under
 *           out/binaries found this 68-byte sequence only at this payload's own
 *           offset 0xEC, and the 0x801F2CA8 sibling differs by one relocation
 *           word, which is shape evidence, not byte identity.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus44AndModeIndex6IfGateElseFFFFWorld00Area00313_801F2CEC(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x44;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}
