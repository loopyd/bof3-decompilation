#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F3590
 * @behavior Loads the shared gate byte at 0x801490C7 with a signed byte read
 *           (`lb`, so the cell is s8); while that byte is zero it stores the
 *           constant 0x72 into the shared status halfword at 0x801490A8
 *           (`addiu v0,0,0x72`, `sh`), and while it is nonzero it stores 0x74
 *           into the same halfword (`addiu v0,0,0x74` in the branch delay slot,
 *           `sh`) and then 6 into the shared mode byte at 0x801D4285
 *           (`addiu v0,0,6`, `sb`); both arms join at one `jr $ra`. No
 *           arguments and no return value; leaf with no frame, and each arm
 *           materialises the `$at` page of its own store site.
 *           Constant/role evidence: the 68-byte span shares its statement
 *           shape, arm order and branch polarity with this target's exact
 *           sibling setStatus60AndModeIndex6IfGateElse5EWorld00Area03713_801F3464
 *           (that body falls through into its zero-gate arm and branches into
 *           its 0x60/mode-6 arm, exactly as this one does), differing only in
 *           the two stored status literals, so the sibling supplied the
 *           statement order, operand forms and declaration style and this
 *           span's own bytes supplied the test and the 0x72/0x74 literals.
 *           One payload scan of the whole 68-byte span found no second copy in
 *           any extracted overlay, so no byte-identical twin exists to clone.
 *           The 0x801F3590 word is the sixth entry of the handler-table run at
 *           0x801F49FC whose other words name the already-lifted siblings at
 *           0x801F3464, 0x801F34A8, 0x801F34D8, 0x801F3508, 0x801F354C and
 *           0x801F35D4, so the entry is a script handler reached only through
 *           that table; the blob carries no prototype, which keeps the argument
 *           roles structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus74AndModeIndex6IfGateElse72World00Area03713_801F3590(void) {
  if (D_801490C7 == 0) {
    D_801490A8 = 0x72;
  } else {
    D_801490A8 = 0x74;
    D_801D4285 = 6;
  }
}
