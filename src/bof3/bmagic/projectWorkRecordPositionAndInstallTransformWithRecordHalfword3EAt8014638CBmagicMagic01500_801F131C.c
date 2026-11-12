#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell 0x1F800044. This target's exact lifts
 * spell the cell D_1F800044 and let the address itself carry the binding, so the
 * declaration needs no map row. The body reads the cell once, before any call,
 * so the original materializes one `lui $v1,0x1F80` / `lw $v1,0x44($v1)` pair and
 * every later access is a field of the loaded pointer. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Battle record pointer published at 0x8014638C in main RAM, outside this
 * overlay image. Splat's generated undefined-syms map already binds the address,
 * and the spelling is the one the other bmagic lifts of this record use. The
 * halfword 0x3E read here is the record field the exact sibling lifts of this
 * body copy into the work record before projecting it. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared screen matrix composed under the freshly built work record transform.
 * Every access is a matrix base handed to MulMatrix2, so the local declaration
 * keeps the object view of the generated map row. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801F131C
 * @behavior Pushes the GTE matrix stack and reads the work record published at
 *           the scratchpad pointer cell 0x1F800044 once. It zeroes a translation
 *           halfword triplet highest element first, then builds a rotation
 *           triplet whose first two elements are work record words 0x34 and 0x38
 *           shifted right by nine and biased by minus 0x4000 and whose third
 *           element is half the negated signed halfword 0x3E of the battle
 *           record read through the main-RAM pointer cell 0x8014638C. RotTrans
 *           projects that triplet into the translation field of a 0x20-byte
 *           scratch matrix while storing a flag word beside it, RotMatrix turns
 *           the zeroed triplet into the rotation of that same matrix, MulMatrix2
 *           composes the shared matrix at 0x801492E8 under it, and the result is
 *           published through SetRotMatrix and SetTransMatrix. Takes no
 *           arguments, returns nothing, and the 0x50-byte frame keeps $ra at
 *           0x4C and $s0 at 0x48 across the matrix calls; the two in-payload
 *           callers pair it with PopMatrix: the emitter dispatch in
 *           dispatchWorkByte2HandlerThenInvokeSharedServiceAndRunSubStepBelow4AndPopMatrixWhenWorkByte9SetBmagicMagic01500_801F0C30
 *           calls it at 0x801F0D20 next to the sibling emitter 0x801F1250, and
 *           func_801F0334 calls it at 0x801F0434.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkRecordPositionAndInstallTransformWithRecordHalfword3EAt8014638CBmagicMagic01500_801F131C(
    void) {
  u8 *work;
  u8 *record;
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
  record = D_8014638C;
  rotation[2] = (s16)(-*(s16 *)(record + 0x3E) / 2);
  RotTrans((SVECTOR *)rotation, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)translation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
