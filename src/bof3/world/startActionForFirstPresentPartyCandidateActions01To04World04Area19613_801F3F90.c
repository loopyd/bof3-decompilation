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
 * original bytes hold 5, 8, 4, 2.
 * @source 0x801F42F4 @kind table
 */
extern u8 D_801F42F4[];
/* Four action halfwords started for the candidate that matched; the original
 * bytes hold 0x01, 0x02, 0x03 and 0x04.
 * @source 0x801F42F8 @kind table
 */
extern s16 D_801F42F8[];

/* Shared front-end action starter of the main executable, outside this payload:
 * it starts the action whose id it is given. Prototype form and argument type
 * follow the exact sibling that reaches it from the same family. */
void func_80150224(s32 arg0);

/* @source 0x801F3F90
 * @behavior Overlay party-scan action starter of the 0x01 to 0x04 action group:
 *           it walks the four candidate bytes of the table at 0x801F42F4 in
 *           order and scans the D_80146254 live 0x140-byte-stride local work
 *           records for one whose party byte (+0x79, at D_80145F09) equals the
 *           candidate. On the first match it starts that candidate action - the
 *           paired halfword entry of the table at 0x801F42F8 through
 *           func_80150224 - then publishes phase 2 in the shared front-end
 *           phase byte D_80143BB0 and returns. With no live record carrying any
 *           of the four candidates it falls out of both loops and returns
 *           without storing. Takes no arguments and answers nothing. The record
 *           count is re-read for the inner loop bottom test because the body
 *           calls out, and the halfword table base is hoisted into a register
 *           before both loops while the byte table access stays the folded
 *           base-plus-index load form. Three facts come from original bytes: the
 *           byte table holds the four candidate bytes 5, 8, 4, 2 and the
 *           halfword table the four action ids 0x01, 0x02, 0x03 and 0x04, and
 *           both sit in this overlay's data blob at 0x801F42F4 and 0x801F42F8.
 *           This body equals the exact party-scan starter of the area148 and
 *           area145 overlays word for word apart from its two table addresses
 *           and the absolute words of its branches, which shift with the span
 *           delta, so that sibling's statement order, operand forms and
 *           declaration style are copied here with this function's own table
 *           bindings. The neighbouring span at 0x801F4054 runs the same scan
 *           over the second table pair at 0x801F4300 and 0x801F4304, whose
 *           bytes hold the 5, 4, 8, 2 candidates and the 0x05 to 0x08 action
 *           ids. This address is published by the overlay's own table word at
 *           0x801F42A4, one of the run the two pointer words at 0x801F42B8 and
 *           0x801F42EC hold the base of, and no jal in the payload reaches it,
 *           so the overlay reaches it as a table handler.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startActionForFirstPresentPartyCandidateActions01To04World04Area19613_801F3F90(
    void) {
  u8 i;
  u8 j;
  u32 offset;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < D_80146254; j++) {
      offset = j * 0x140;
      if (D_80145F09[offset] == D_801F42F4[i]) {
        func_80150224(D_801F42F8[i]);
        D_80143BB0 = 2;
        return;
      }
    }
  }
}
