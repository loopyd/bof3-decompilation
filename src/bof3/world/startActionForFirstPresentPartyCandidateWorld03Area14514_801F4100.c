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

/* Four candidate party bytes compared against the work-record party byte.
 * @source 0x801F5CA8 @kind table
 */
extern u8 partyCandidateByteTableWorld03Area14514_801F5CA8[];
/* Four action halfwords started for the candidate that matched.
 * @source 0x801F5CAC @kind table
 */
extern s16 partyCandidateActionTableWorld03Area14514_801F5CAC[];

/* Shared front-end action starter of the main executable, outside this payload:
 * it starts the action whose id it is given. Prototype form and argument type
 * follow the exact sibling that reaches it from the same family. */
void func_80150224(s32 arg0);

/* @source 0x801F4100
 * @behavior Overlay party-scan action starter: it walks the four candidate
 *           bytes of the table at 0x801F5CA8 in order and scans the D_80146254
 *           live 0x140-byte-stride local work records for one whose party byte
 *           (+0x79, at D_80145F09) equals the candidate. On the first match it
 *           starts that candidate action - the paired halfword entry of the
 *           table at 0x801F5CAC through func_80150224 - then publishes phase 2
 *           in the shared front-end phase byte D_80143BB0 and returns. With no
 *           live record carrying any of the four candidates it falls out of
 *           both loops and returns without storing. Takes no arguments and
 *           answers nothing. The record count is re-read for the inner loop
 *           bottom test because the body calls out, and the halfword table base
 *           is hoisted into a register before both loops while the byte table
 *           access stays the folded base-plus-index load form. Three facts come
 *           from original bytes: the tables hold the four candidate bytes 5, 8,
 *           2, 4 and the four action halfwords 5, 6, 7, 8; both sit in this
 *           overlay data at 0x801F5CA8 and 0x801F5CAC. The body matches the
 *           exact party-scan starter of the scena14 overlay at 0x801FA904 in
 *           statement order and loop shape, differing in that the tables are
 *           this overlay data rather than caller arguments and the answer is
 *           empty rather than a returned action id. No image word holds this
 *           address, so the overlay reaches it from outside its payload and its
 *           prologue and return paths alone place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startActionForFirstPresentPartyCandidateWorld03Area14514_801F4100(void) {
  u8 i;
  u8 j;
  u32 offset;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < D_80146254; j++) {
      offset = j * 0x140;
      if (D_80145F09[offset] ==
          partyCandidateByteTableWorld03Area14514_801F5CA8[i]) {
        func_80150224(partyCandidateActionTableWorld03Area14514_801F5CAC[i]);
        D_80143BB0 = 2;
        return;
      }
    }
  }
}
