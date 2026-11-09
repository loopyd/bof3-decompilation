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
 * original bytes hold 8, 5, 2, 4.
 * @source 0x801F5CB4 @kind table
 */
extern u8 partyCandidateByteTableWorld03Area14514_801F5CB4[];
/* Four action halfwords started for the candidate that matched; the original
 * bytes hold 0x0B, 0x0C, 0x0D and 0x0E.
 * @source 0x801F5CB8 @kind table
 */
extern s16 partyCandidateActionTableWorld03Area14514_801F5CB8[];

/* Shared front-end action starter of the main executable, outside this payload:
 * it starts the action whose id it is given. Prototype form and argument type
 * follow the exact sibling that reaches it from the same family. */
void func_80150224(s32 arg0);

/* @source 0x801F41C4
 * @behavior Overlay party-scan action starter of the 0x0B to 0x0E action group:
 *           it walks the four candidate bytes of the table at 0x801F5CB4 in
 *           order and scans the D_80146254 live 0x140-byte-stride local work
 *           records for one whose party byte (+0x79, at D_80145F09) equals the
 *           candidate. On the first match it starts that candidate action - the
 *           paired halfword entry of the table at 0x801F5CB8 through
 *           func_80150224 - then publishes phase 2 in the shared front-end
 *           phase byte D_80143BB0 and returns. With no live record carrying any
 *           of the four candidates it falls out of both loops and returns
 *           without storing. Takes no arguments and answers nothing. The record
 *           count is re-read for the inner loop bottom test because the body
 *           calls out, and the halfword table base is hoisted into a register
 *           before both loops while the byte table access stays the folded
 *           base-plus-index load form. Three facts come from original bytes: the
 *           byte table holds the four candidate bytes 8, 5, 2, 4 and the
 *           halfword table the four action ids 0x0B, 0x0C, 0x0D and 0x0E. This
 *           body equals the party-scan starter of the 0x801F4100 sibling word
 *           for word apart from its two table addresses, which sit 0xC higher,
 *           and the absolute word of the store-then-return jump, which shifts
 *           by the 0xC4 span delta; the sibling was exact at the time this seed
 *           was written, so its statement order, operand forms and declaration
 *           style are copied and this function carries its own table bindings.
 *           The four starters at 0x801F403C, 0x801F4100, 0x801F41C4 and
 *           0x801F4288 fill the consecutive code-pointer words
 *           0x801F5C34-0x801F5C40 of the run at 0x801F5C20, and the three
 *           candidate tables at 0x801F5C9C, 0x801F5CA8 and 0x801F5CB4 are
 *           consecutive 0xC-byte records of the payload text blob at 0x801F568C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startActionForFirstPresentPartyCandidateActions0BTo0EWorld03Area14514_801F41C4(
    void) {
  u8 i;
  u8 j;
  u32 offset;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < D_80146254; j++) {
      offset = j * 0x140;
      if (D_80145F09[offset] ==
          partyCandidateByteTableWorld03Area14514_801F5CB4[i]) {
        func_80150224(partyCandidateActionTableWorld03Area14514_801F5CB8[i]);
        D_80143BB0 = 2;
        return;
      }
    }
  }
}
