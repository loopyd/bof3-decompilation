#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F2C04
 * @behavior Loads the shared gate byte at 0x801490C7 with a signed byte read
 *           (`lb`, so the cell is s8); while that byte is zero it stores the
 *           constant 0x42 into the shared status halfword at 0x801490A8, and
 *           while it is nonzero it stores 0x44 into the same halfword (the 0x44
 *           is materialised in the branch delay slot, `sh`) and then 6 into the
 *           shared mode byte at 0x801D4285 (`addiu v0,0,6`, `sb`); both arms
 *           join at one `jr $ra`. No arguments and no return value; leaf with
 *           no frame, and each arm materialises the `$at` page of its own store
 *           site.
 *         Shape: a word-by-word comparison of this 68-byte span against the
 *           0x801F3590 body of World00 Area037 EMI#13 (one payload word scan,
 *           not a lift status) found exactly three differing words - the two
 *           stored status literals 0x44/0x42 against that body's 0x74/0x72,
 *           and the internal `j` immediate, which moves by 0x263, the span
 *           delta 0x98C shifted for the encoded target - so every relative
 *           branch, operand form, offset spelling and cell reference is equal.
 *           That body's exact source supplied the statement order, the arm
 *           order and the branch polarity; this span's own bytes supplied the
 *           0x42/0x44 literals. The payload scan also found the 68-byte
 *           sequence only at this payload's own offset 4, so no byte-identical
 *           twin exists to clone. This target's sibling at 0x801F2CA8 carries
 *           the same 0x44/mode-6 arm with the opposite branch polarity and a
 *           0xFFFF else, which is shape evidence only.
 *         Boundary and role: the only reference to this address in the payload
 *           is the address-taken word at offset 0x72C (0x801F332C) of the tail
 *           blob, with no `jal` immediate anywhere, so its role stays
 *           address-taken and the script blob carries no prototype, which keeps
 *           the empty parameter list structural. The body ends with its own
 *           `jr $ra` and the 0x44-byte span ends exactly at the next run entry
 *           0x801F2C48.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateModeGateElse42(void) {
  if (D_801490C7 == 0) {
    D_801490A8 = 0x42;
  } else {
    D_801490A8 = 0x44;
    D_801D4285 = 6;
  }
}
