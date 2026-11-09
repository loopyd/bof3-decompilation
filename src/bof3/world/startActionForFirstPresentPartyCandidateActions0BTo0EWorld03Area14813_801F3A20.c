#include "bof3/bof3.h"

/* Shared live count of the 0x140-byte-stride local work records.
 * @source 0x80146254 @kind unknown
 */
extern u8 D_80146254;
/* Party byte (+0x79) of the 0x140-byte-stride local work records based at
 * 0x80145E90. The original rebuilds the row base for this access, so the byte
 * keeps its own absolute symbol instead of a record view.
 * @source 0x80145F09 @kind unknown
 */
extern u8 D_80145F09[];
/* Shared front-end phase byte.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Four candidate party bytes compared against the work-record party byte; the
 * original bytes hold 5, 2, 8, 4.
 * @source 0x801F6144 @kind table
 */
extern u8 D_801F6144[];
/* Four action halfwords started for the candidate that matched; the original
 * bytes hold 0x0B, 0x0C, 0x0D and 0x0E.
 * @source 0x801F6148 @kind table
 */
extern s16 D_801F6148[];

/* Shared front-end action starter of the main executable, outside this payload:
 * it starts the action whose id it is given. Prototype form and argument type
 * follow the exact sibling that reaches it from the same family. */
void func_80150224(s32 arg0);

/* @source 0x801F3A20
 * @behavior Overlay party-scan action starter of the 0x0B to 0x0E action group:
 *           it walks the four candidate bytes of the table at 0x801F6144 in
 *           order and scans the D_80146254 live 0x140-byte-stride local work
 *           records for one whose party byte (+0x79, at D_80145F09) equals the
 *           candidate. On the first match it starts that candidate action - the
 *           paired halfword entry of the table at 0x801F6148 through
 *           func_80150224 - then publishes phase 2 in the shared front-end
 *           phase byte D_80143BB0 and returns. With no live record carrying any
 *           of the four candidates it falls out of both loops and returns
 *           without storing. Takes no arguments and answers nothing. The record
 *           count is re-read for the inner loop bottom test, and the halfword
 *           table base is hoisted into a register before both loops while the
 *           byte table access stays the folded base-plus-index load form. Three
 *           facts come from original bytes: the byte table holds the four
 *           candidate bytes 5, 2, 8, 4 and the halfword table the four action
 *           ids 0x0B, 0x0C, 0x0D and 0x0E, and both sit in this overlay's data
 *           blob at 0x801F6144 and 0x801F6148. This body equals the exact
 *           party-scan starter of the area145 overlay at 0x801F41C4 word for
 *           word apart from its two table addresses and the absolute words of
 *           its branches, which shift with the span delta; that sibling was
 *           exact, so its statement order, operand forms and declaration style
 *           are copied and this function carries its own table bindings. The
 *           paired starter at 0x801F3AE4 runs the same scan over the second
 *           candidate table at 0x801F6150 and the 0x0F to 0x12 action group at
 *           0x801F6154. No image word holds this address, so the overlay
 *           reaches it from outside its payload and its prologue and return
 *           paths alone place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startActionForFirstPresentPartyCandidateActions0BTo0EWorld03Area14813_801F3A20(
    void) {
  u8 i;
  u8 j;
  u32 offset;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < D_80146254; j++) {
      offset = j * 0x140;
      if (D_80145F09[offset] == D_801F6144[i]) {
        func_80150224(D_801F6148[i]);
        D_80143BB0 = 2;
        return;
      }
    }
  }
}
