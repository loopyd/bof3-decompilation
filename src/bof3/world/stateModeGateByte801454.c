#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;
/* @source 0x801454EC @kind unknown */
extern u8 D_801454EC;

/* @source 0x801F2C04
 * @behavior Loads the shared gate byte D_801490C7; while that byte is nonzero
 *           the shared status halfword D_801490A8 is stored with 4. When the
 *           gate byte is zero it loads the shared byte D_801454EC and, while
 *           that byte is below 6, stores 3 into D_801490A8; a byte of 6 or more
 *           stores 2 into D_801490A8 instead and touches nothing else. The two
 *           4/3 paths join and then store 6 into the shared mode index byte
 *           D_801D4285, so that store runs once after the branch join; the 2
 *           path leaves the mode index alone. Takes no arguments and returns
 *           nothing; leaf with no frame and no call, each status constant is
 *           materialised in $v0 and fills the branch delay slot of the test
 *           that selects it, and the joined store reads that same $v0.
 *           Shape: a payload cmp of the 0x5C bytes at 0x801F2C04 against
 *           out/binaries/emi/world01/area050/13.bin offset 0x4 came back equal
 *           from the first word to the `jr $ra` nop except five words - the
 *           three status literals (4/3/2 here against 0x84/0x83/0x82 there),
 *           the `sltiu` bound (6 here against 0x1E there) and the `lbu` offset
 *           word of the tested cell (0x54EC here against 0x54F3 there, whose
 *           `lui` %hi word is shared and therefore equal) - so every branch and
 *           the internal `j` target stay equal and the two spans are one body.
 *           That recorded-exact sibling
 *           setStatus84Or83AndModeIndex6IfGateOrFahlBattleCountElse82World01Area05013_801F2C04
 *           supplied this body's statement order, operand forms, offset
 *           spelling and declaration style; this span's own bytes supplied its
 *           literals, its bound and its tested cell. A whole-payload scan of
 *           out/binaries for this span's first 14 words returned this payload
 *           alone, so no byte-identical sibling exists for it.
 *           The five-word handler pointer run at payload offset 0x1B8
 *           (0x801F2C04, 0x801F2C60, 0x801F2C90, 0x801F2CC0, 0x801F2D04) carries
 *           this address as its first word, and the payload's only `jal`
 *           (func_8015C088, from initArea11313State) does not target
 *           0x801F2C04, so the payload installs the body as a table member
 *           rather than reaching it with a call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateModeGateByte801454(
    void) {
  u16 status;

  if (D_801490C7 != 0) {
    status = 4;
  } else if (D_801454EC < 6) {
    status = 3;
  } else {
    D_801490A8 = 2;
    return;
  }
  D_801490A8 = status;
  D_801D4285 = 6;
}
