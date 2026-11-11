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
 * original bytes hold 2, 4, 8, 5.
 * @source 0x801F4300 @kind table
 */
extern u8 D_801F4300[];
/* Four action halfwords started for the candidate that matched; the original
 * bytes hold 0x05, 0x06, 0x07 and 0x08.
 * @source 0x801F4304 @kind table
 */
extern s16 D_801F4304[];

/* Shared front-end action starter of the main executable, outside this payload:
 * it starts the action whose id it is given. Prototype form and argument type
 * follow the exact sibling that reaches it from the same family. */
void func_80150224(s32 arg0);

/* @source 0x801F4054
 * @behavior Overlay party-scan action starter of the 0x05 to 0x08 action group:
 *           it walks the four candidate bytes of the table at 0x801F4300 in
 *           order and scans the D_80146254 live 0x140-byte-stride local work
 *           records for one whose party byte (+0x79, at D_80145F09) equals the
 *           candidate. On the first match it starts that candidate action - the
 *           paired halfword entry of the table at 0x801F4304 through
 *           func_80150224 - then publishes phase 2 in the shared front-end
 *           phase byte D_80143BB0 and returns. With no live record carrying any
 *           of the four candidates it falls out of both loops and returns
 *           without storing. Takes no arguments and answers nothing. The record
 *           count is re-read for the inner loop bottom test because the body
 *           calls out, and the halfword table base is hoisted into a register
 *           before both loops while the byte table access stays the folded
 *           base-plus-index load form. Four facts come from this span's original
 *           bytes: the byte table holds the four candidate bytes 2, 4, 8 and 5,
 *           the halfword table the four action ids 0x05, 0x06, 0x07 and 0x08,
 *           and both sit in this overlay's data blob at 0x801F4300 and
 *           0x801F4304, at +0x1D4 and +0x1D8 of the blob base at 0x801F412C.
 *           The neighbouring span at 0x801F3F90 runs the same scan over the
 *           first table pair at 0x801F42F4 and 0x801F42F8 and the 0x01 to 0x04
 *           action group, and it was exact when this seed was written; its
 *           statement order, operand forms and declaration style are copied
 *           here with this function's own table bindings. The two bodies are not
 *           byte-identical - each carries its own table addresses, which here
 *           sit 0xC above the neighbour's while the span delta is 0xC4, so the
 *           copy is shape evidence and the bodies were written from this span's
 *           own bytes. This address is published by the overlay's own table word
 *           at 0x801F42A8, the second entry of the four-word handler run at
 *           0x801F42A4 whose base the pointer words at 0x801F42B8 and 0x801F42EC
 *           hold, and no jal in the payload reaches it, so the overlay reaches
 *           it as a table handler.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startActionForFirstPresentPartyCandidateActions05To08World04Area19613_801F4054(
    void) {
  u8 i;
  u8 j;
  u32 offset;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < D_80146254; j++) {
      offset = j * 0x140;
      if (D_80145F09[offset] == D_801F4300[i]) {
        func_80150224(D_801F4304[i]);
        D_80143BB0 = 2;
        return;
      }
    }
  }
}
