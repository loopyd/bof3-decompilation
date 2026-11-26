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
 * work-record party byte; the original bytes hold 8, 4, 2 and 5.
 * @source 0x801F4A44 @kind table
 */
extern u8 D_801F4A44[];
/* Front-end action id paired with each candidate party byte of the table at
 * 0x801F4A44 (the original bytes hold 0x0A, 0x0B, 0x0C and 0x0D) and started for
 * the candidate that matched.
 * @source 0x801F4A48 @kind table
 */
extern s16 D_801F4A48[];

void func_80150224(s32 arg0);

/* @source 0x801F3B68
 * @behavior Overlay party-scan action starter of the 0x0A to 0x0D action group:
 *           it walks the four candidate party bytes of the local table at
 *           0x801F4A44 in order and, for each candidate, scans the D_80146254 live
 *           0x140-byte-stride local work records for one whose party byte
 *           (+0x79, at D_80145F09) equals it. On the first match it starts that
 *           candidate's front-end action - the paired halfword entry of the
 *           local table at 0x801F4A48 through func_80150224 - publishes phase 2
 *           in the shared front-end phase byte D_80143BB0 and returns, so the
 *           lowest candidate index whose party is present wins and no scan runs
 *           at all while the record count is zero. The record count is read once
 *           for the shared inner entry test and re-read for each candidate
 *           pass because the body calls out. The inner loop keeps the record
 *           offset in its own local so the D_80145F09 access stays the folded
 *           lui/addu/lbu form rather than a hoisted table base, while the
 *           paired action entry stays a loop-invariant pointer in a saved
 *           register. Nothing in this payload names this body: no jal
 *           immediate and no pointer word holds its address, so it is an
 *           overlay entry reached from outside the payload and its parameter
 *           roles stay structural. Its statement order, operand forms and
 *           declaration style follow the exact work-record scans at
 *           0x801F56B0 and 0x801FA904, whose own record-access form and
 *           candidate-table pair are the same shape; the six 196-byte handler
 *           slots of this payload that read the same +0x79 byte from their own
 *           table pair are still asm rows, so they are shape evidence and not
 *           clone proof, and this body writes its own literals - the four-entry
 *           candidate and action tables, the stride 0x140 and the phase 2 - from
 *           its own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startActionFirstPresent(
    void) {
  u8 i;
  u8 j;
  u32 offset;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < D_80146254; j++) {
      offset = j * 0x140;
      if (D_80145F09[offset] == D_801F4A44[i]) {
        func_80150224(D_801F4A48[i]);
        D_80143BB0 = 2;
        return;
      }
    }
  }
}
