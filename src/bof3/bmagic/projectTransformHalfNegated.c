#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell 0x1F800044, bound by this target's own map
 * row g_battle_work. This body reads the cell once, before any call, so the
 * original materializes one folded `lui $v1,0x1F80` / `lw $v1,0x44($v1)` pair and
 * every later access is a field of the loaded pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared screen matrix composed under the freshly built work-record transform.
 * Its only access is a matrix base handed to MulMatrix2, so the local declaration
 * keeps the object view of the row the target's generated undefined_syms_auto.txt
 * already binds. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* Distance helper of the shared main executable: it takes a signed 16.16 word
 * pair and answers their distance as a halfword while the answer arrives in a
 * full word, which is why this body sign-extends it before negating it. The
 * generated undefined_funcs_auto.txt binds the address, so the prototype carries
 * no map row.
 * @source 0x8015477C
 */
extern s32 func_8015477C(s32 x, s32 y);

/* @source 0x801EF8E8
 * @behavior Pushes the GTE matrix stack and reads the work record published at
 *           the scratchpad pointer cell 0x1F800044 once, before any call. Zeroes
 *           a translation halfword triplet in ascending element order, then
 *           builds a rotation triplet whose first two elements are work record
 *           words 0x34 and 0x38 shifted right by nine and biased by minus
 *           0x4000, and whose third element is half the negated work record
 *           signed halfword 0x3E when work byte 0x2 is below three and half the
 *           negated distance the shared main-executable helper 0x8015477C
 *           answers for that same word pair otherwise. RotTrans projects the
 *           triplet into the translation field of a 0x20-byte scratch matrix
 *           while storing a flag word beside it, RotMatrix turns the zeroed
 *           triplet into the rotation of that same matrix, MulMatrix2 composes
 *           the shared matrix at 0x801492E8 under it, and the result is
 *           published through SetRotMatrix and SetTransMatrix. Takes no
 *           arguments and returns nothing; the 0x50-byte frame keeps $ra at 0x4C
 *           and $s0 at 0x48 across the matrix calls. No payload jal word and no
 *           payload data word names this address, so an out-of-payload or
 *           overlay-loader caller reaches it and its roles stay structural. The
 *           0xEC-byte span ends at the next declared subsegment, 0x801EF9D4. The
 *           body was not cloned: masked and opcode/register payload scans over
 *           every extracted binary returned only this span's own self-match, so
 *           the statement order came from the family's exact lifts, with this
 *           span's own guard, constants and translation store order written from
 *           its own bytes. A seed that kept one shared angle local divided after
 *           the join measured 55 of 59 instructions, its two negations landing
 *           in $v1 where the original negates in place in $v0; halving the value
 *           inside each arm measured 59 of 59 at 236 bytes, because the jump
 *           pass merges those identical arm tails.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectTransformHalfNegated(
    void) {
  u8 *work;
  s16 rotation[3];
  s16 translation[3];
  MATRIX object_work;
  long flag;

  PushMatrix();

  work = g_battle_work;
  translation[0] = 0;
  translation[1] = 0;
  translation[2] = 0;
  rotation[0] = (s16)(*(s32 *)(work + 0x34) >> 9) - 0x4000;
  rotation[1] = (s16)(*(s32 *)(work + 0x38) >> 9) - 0x4000;
  if (work[2] < 3) {
    rotation[2] = (s16)(-*(s16 *)(work + 0x3E) / 2);
  } else {
    rotation[2] = (s16)(-(s16)func_8015477C(*(s32 *)(work + 0x34),
                                            *(s32 *)(work + 0x38)) / 2);
  }
  RotTrans((SVECTOR *)rotation, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)translation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
