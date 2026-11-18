#include "bof3/bof3.h"

/* @source 0x80146254 @kind unknown — shared live count of the 0x140-byte-stride
 * local work records. */
extern u8 D_80146254;
/* @source 0x80145F09 @kind unknown — party byte (+0x79) of the 0x140-byte-stride
 * local work records based at 0x80145E90. */
extern u8 D_80145F09[];
/* @source 0x80143BB0 @kind unknown — shared front-end phase byte. */
extern u8 D_80143BB0;

/* Four candidate party bytes of this handler slot compared against the
 * work-record party byte; the original bytes hold 4, 5, 8 and 2.
 * @source 0x801F4A74 @kind table
 */
extern u8 D_801F4A74[];
/* Front-end action id paired with each candidate party byte of the table at
 * 0x801F4A74 (the original bytes hold 0x1E, 0x1F, 0x20 and 0x21) and started
 * for the candidate that matched.
 * @source 0x801F4A78 @kind table
 */
extern s16 D_801F4A78[];

void func_80150224(s32 arg0);

/* @source 0x801F3E78
 * @behavior Overlay party-scan action starter of the 0x1E to 0x21 action group:
 *           it walks the four candidate party bytes of the local table at
 *           0x801F4A74 in order and, for each candidate, scans the D_80146254
 *           live 0x140-byte-stride local work records for one whose party byte
 *           (+0x79, at D_80145F09) equals it. On the first match it starts that
 *           candidate's front-end action - the paired halfword entry of the
 *           local table at 0x801F4A78 through func_80150224 - publishes phase 2
 *           in the shared front-end phase byte D_80143BB0 and returns, so the
 *           lowest candidate index whose party is present wins and no scan runs
 *           at all while the record count is zero. The record count is read once
 *           for the shared inner entry test and re-read for each candidate pass
 *           because the body calls out. The inner loop keeps the record offset
 *           in its own local so the D_80145F09 access stays the folded
 *           lui/addu/lbu form rather than a hoisted table base, while the paired
 *           action entry stays a loop-invariant pointer in a saved register.
 *           The four candidate bytes and the four action ids come from this
 *           span's own bytes: both tables sit in this overlay's data blob, at
 *           +0x78 and +0x7C of the textbin run declared at 0x801F49FC. Nothing
 *           in this payload reaches this body through a jal immediate; its
 *           address is published by the pointer word at 0x801F49D8, the sixth
 *           entry of the handler run at 0x801F49C4, so it is a table handler and
 *           its parameter roles stay structural. Its statement order, operand
 *           forms and declaration style follow the handler slot at 0x801F3B68,
 *           which was exact when this seed was written: a comparison of the two
 *           196-byte payload spans differs in exactly three words - the two
 *           addiu/lo halves of this slot's own candidate and action table
 *           literals, each 0x30 above the sibling's, and the tail j immediate,
 *           which is 0x310 further on, the span delta between the two slots -
 *           so the bodies are one shape and this one writes its own literals
 *           from its own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startActionForFirstPresentPartyCandidateActions1ETo21World04Area19713_801F3E78(
    void) {
  u8 i;
  u8 j;
  u32 offset;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < D_80146254; j++) {
      offset = j * 0x140;
      if (D_80145F09[offset] == D_801F4A74[i]) {
        func_80150224(D_801F4A78[i]);
        D_80143BB0 = 2;
        return;
      }
    }
  }
}
