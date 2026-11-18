#include "bof3/bof3.h"

/*
 * Scratchpad work-record pointer cell 0x1F800044. This target's map owns the
 * address as g_battle_work, and the body reads it once, before any call, so the
 * original materializes a single lui $v1,0x1F80 / lw $v1,0x44($v1) pair and
 * every later access is a field of the loaded pointer.
 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/*
 * Shared screen matrix composed under the freshly built work-record transform.
 * Its only access is a matrix base handed to MulMatrix2; the remaining asm of
 * this overlay references the address, so Splat's generated
 * undefined_syms_auto.txt already binds it and the target map needs no row.
 */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801EF434
 * @behavior Pushes the GTE matrix stack and reads the scratchpad work-record
 *           pointer cell once, before any call. Zeroes a translation halfword
 *           triplet in ascending element order, then builds a rotation triplet
 *           whose first two elements are work record words 0x34 and 0x38 shifted
 *           right by nine and biased by minus 0x4000, and whose third element is
 *           half the negated sum of the record signed halfword 0x3E and 0x100.
 *           RotTrans projects that triplet into the translation field of a
 *           0x20-byte scratch matrix and writes a flag word after it, RotMatrix
 *           turns the zeroed triplet into that same matrix rotation, MulMatrix2
 *           composes the shared matrix at 0x801492E8 with it, and the result is
 *           installed through SetRotMatrix and SetTransMatrix. Takes no
 *           arguments and returns nothing; the 0x50-byte frame keeps the return
 *           address at 0x4C and a saved register at 0x48 across the matrix
 *           calls. Its one in-payload call at 0x801EEEFC passes no arguments and
 *           pairs the transform with PopMatrix before the two draw steps
 *           0x801EF8E8 and 0x801EF9D4. The nearest sibling is the already-exact
 *           0x801EF1E4 of emi/bmagic/magic144/03, whose 0xB0-byte body is these
 *           bytes with the 0x100 bias absent; no extracted payload holds a
 *           byte-identical copy of this 0xB4-byte span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkRecordPositionAndInstallTransformWithZeroRotationAndHalfNegatedRecordHalfword3EPlus100BmagicMagic10003_801EF434(
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
  rotation[2] = (s16)(-(*(s16 *)(work + 0x3E) + 0x100) / 2);
  RotTrans((SVECTOR *)rotation, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)translation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
