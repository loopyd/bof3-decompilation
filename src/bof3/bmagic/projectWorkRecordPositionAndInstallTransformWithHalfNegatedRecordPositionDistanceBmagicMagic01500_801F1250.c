#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell 0x1F800044. This target's exact lifts
 * spell the cell D_1F800044 and let the address itself carry the binding, so the
 * declaration needs no map row. The body reads the cell once, before any call,
 * so the original materializes one `lui $v1,0x1F80` / `lw $v1,0x44($v1)` pair and
 * every later access is a field of the loaded pointer. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared screen matrix composed under the freshly built work record transform.
 * Every access is a matrix base handed to MulMatrix2, so the local declaration
 * keeps the object view of the generated map row. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* Distance helper of the shared main executable: it takes a signed 16.16 word
 * pair and answers their distance as a halfword, while the answer arrives in a
 * full word, which is why this body sign-extends it before negating it. The
 * generated undefined-funcs map binds the address, so the prototype carries no
 * map row.
 * @source 0x8015477C
 */
extern s32 func_8015477C(s32 x, s32 y);

/* @source 0x801F1250
 * @behavior Emitter step of this overlay, the body
 *           dispatchWorkByte2HandlerThenInvokeSharedServiceAndRunSubStepBelow4AndPopMatrixWhenWorkByte9SetBmagicMagic01500_801F0C30
 *           runs when work byte 2 is below three, beside the sibling emitter
 *           0x801F131C. It pushes the GTE matrix stack and reads the work record
 *           published at the scratchpad pointer cell 0x1F800044 once, before any
 *           call. It zeroes a translation halfword triplet highest element first,
 *           then builds a rotation triplet whose first two elements are work
 *           record words 0x34 and 0x38 shifted right by nine and biased by minus
 *           0x4000 and whose third element is half the negated distance the
 *           shared main-executable distance helper 0x8015477C answers for that
 *           same word pair. RotTrans projects that triplet into the translation
 *           field of a 0x20-byte scratch matrix while storing a flag word beside
 *           it, RotMatrix turns the zeroed triplet into the rotation of that same
 *           matrix, MulMatrix2 composes the shared matrix at 0x801492E8 under it,
 *           and the result is published through SetRotMatrix and SetTransMatrix.
 *           Takes no arguments and returns nothing; the 0x50-byte frame keeps
 *           $ra at 0x4C and $s0 at 0x48 across the matrix calls. Both in-payload
 *           call sites pass no arguments and pair it with PopMatrix: 0x801F0D10
 *           in the work byte 2 dispatch above and 0x801F0424 in the unlifted
 *           handler func_801F0334. No payload jal or table word publishes its
 *           address, and the 0xCC-byte span ends at the next subsegment,
 *           0x801F131C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkRecordPositionAndInstallTransformWithHalfNegatedRecordPositionDistanceBmagicMagic01500_801F1250(
    void) {
  u8 *work;
  s16 rotation[3];
  s16 translation[3];
  MATRIX object_work;
  long flag;

  PushMatrix();

  work = D_1F800044;
  translation[2] = 0;
  translation[1] = 0;
  translation[0] = 0;
  rotation[0] = (s16)(*(s32 *)(work + 0x34) >> 9) - 0x4000;
  rotation[1] = (s16)(*(s32 *)(work + 0x38) >> 9) - 0x4000;
  rotation[2] =
      -(s16)func_8015477C(*(s32 *)(work + 0x34), *(s32 *)(work + 0x38)) / 2;
  RotTrans((SVECTOR *)rotation, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)translation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
