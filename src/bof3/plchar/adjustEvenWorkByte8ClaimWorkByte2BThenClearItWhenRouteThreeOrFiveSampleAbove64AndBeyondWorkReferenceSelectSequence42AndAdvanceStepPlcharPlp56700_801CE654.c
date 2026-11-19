#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell before every block of the body and again before
 * each store, because a store through the pointer may alias the cell itself, so
 * the source keeps the named-symbol form and evaluates it again at every
 * access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Scratchpad gate byte of the shared main executable: the body runs a layer
 * query only while this byte reads nonzero.
 * @source 0x1F800000 @kind data
 */
extern u8 D_1F800000[];

/* Layer probe offset words of the shared main-executable data section: one
 * first-coordinate and one second-coordinate offset per probed layer, added to
 * the work words 0x34 and 0x38. The original loads each of them as a whole word
 * with no index scaling, so they are scalars rather than table entries.
 * @source 0x80181BAC @kind data
 */
extern s32 D_80181BAC;
/* @source 0x80181BB0 @kind data */
extern s32 D_80181BB0;
/* @source 0x80181BBC @kind data */
extern s32 D_80181BBC;
/* @source 0x80181BC0 @kind data */
extern s32 D_80181BC0;

/* Route-parameterised position routine of the shared EXE image, outside this
 * payload: it takes the two coordinate words and a zero-extended route byte and
 * answers the sample the body compares against 0x41. The lift
 * claimWorkByte2BThenClearItWhenRouteThreeOrFiveSampleAbove64AndBeyondWorkReferencePlcharPlp56700_801CF944
 * of this target declares the same prototype. */
s32 func_80154F78(s32 arg0, s32 arg1, u8 arg2);

/* Screen-space position routine of the shared EXE image, outside this payload:
 * it takes the two coordinate words and answers the signed halfword sample the
 * body compares with the work reference halfword 0x3E. That same sibling lift
 * declares the same prototype. */
s16 func_80154F28(s32 x, s32 y);

/* Shared EXE-side work-sequence selector; the original narrows the computed
 * sequence id with `srl` and adds the bias 0x42 without an `andi`, so the call
 * carries the byte-wide argument the sibling lift
 * adjustEvenWorkByte8ClaimFrontEndRecordHandler1BSelectSequenceAndQueueCueThenAdvanceWorkByte2PlcharPlp47800_801CF4CC
 * also declares. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CE654
 * @behavior Entry 0 of the handler run based at 0x801D0798, which this
 *           payload's byte-2 dispatcher 0x801CE610 indexes with work byte 2
 *           scaled by four, so the body runs while work byte 2 reads zero. It
 *           acts on the front-end work object read through the scratchpad
 *           pointer cell 0x1F800044. The body first rewinds that object's step
 *           byte 0x08 by one modulo 8, but only while bit 0 of the byte is
 *           clear, so an odd byte holds. It then sets the claim byte 0x2B to 1
 *           and probes two fixed route offsets of the shared main-executable
 *           data section: 0x80181BAC/0x80181BB0 with route 3 and
 *           0x80181BBC/0x80181BC0 with route 5. For each probe it projects the
 *           work position by adding that offset pair to the work words 0x34 and
 *           0x38, asks the route-parameterised position routine func_80154F78
 *           for the sample of that point, and while the scratchpad gate byte
 *           0x1F800000 reads nonzero and that sample narrowed to 16 bits is at
 *           least 0x41 it asks the position routine func_80154F28 for the same
 *           point and clears the claim byte when the reference halfword 0x3E is
 *           below that answer, so the two probes only ever clear it. It then
 *           selects the shared work sequence (step byte 0x08 >> 1) + 0x42
 *           through func_8014D8D4 and advances the work state byte 0x02 by one.
 *           It takes no arguments and returns nothing; the 0x20-byte frame only
 *           keeps $ra at 0x18 and the two projected coordinates at 0x14 and
 *           0x10 across the calls. The route constants 3 and 5 are materialised
 *           in the argument register per probe, and the 5 of the second probe
 *           sits in the branch delay slots of the first block.
 *
 * Evidence: the 0x1CC-byte span is one complete function of the payload - the
 * scheduled prologue reads the 0x1F800044 cell before `addiu $sp,-0x20`, the
 * body holds four `jal`s and the single epilogue `lw $ra,0x18($sp); addiu
 * $sp,$sp,0x20; jr $ra; nop`. No `jal` in this payload names it; its only
 * in-image reference is the word at 0x801D0798, which holds 0x801CE654, so the
 * overlay loader that steps the run naming it is its caller.
 *
 * Shape evidence: the statement order, operand forms, offset spelling and
 * declaration style of the two probe blocks come from the exact sibling
 * claimWorkByte2BThenClearItWhenRouteThreeOrFiveSampleAbove64AndBeyondWorkReferencePlcharPlp56700_801CF944
 * of this target, whose body differs from this one in the step-byte rewind
 * before it and the sequence selection and state advance after it; the rewind
 * block and the `(byte 0x08 >> 1) + 0x42` selector form follow the exact
 * sibling adjustEvenWorkByte8ClaimFrontEndRecordHandler1BSelectSequenceAndQueueCueThenAdvanceWorkByte2PlcharPlp47800_801CF4CC,
 * and the advance of work state byte 0x02 is this span's own last store. This
 * span's literals, offsets and route constants are read from its own bytes. The
 * whole 0x1CC-byte span also compares byte-identical, by payload bytes rather
 * than by any row's metadata, to the span at 0x801CE654 of plp578/00 and to the
 * spans at 0x801CEE4C of plp257/00, 0x801CF37C of plp056/00, 0x801CF750 of
 * plp015/00, 0x801CE870 of plp457/00 and 0x801CFB74 of plp025/00, so the body
 * is one shared family body and not this overlay's own variant. Matched at the
 * first gate, so no contrast against a losing spelling was measured.
 * @status exact
 * @match 100.00
 * @residual none
 */
void adjustEvenWorkByte8ClaimWorkByte2BThenClearItWhenRouteThreeOrFiveSampleAbove64AndBeyondWorkReferenceSelectSequence42AndAdvanceStepPlcharPlp56700_801CE654(
    void) {
  s32 x;
  s32 y;
  s32 sample;

  if ((D_1F800044[0x8] & 1u) == 0u) {
    D_1F800044[0x8] = (u8)((D_1F800044[0x8] - 1) & 7u);
  }

  D_1F800044[0x2b] = 1;

  x = *(s32*)(D_1F800044 + 0x34) + D_80181BAC;
  y = *(s32*)(D_1F800044 + 0x38) + D_80181BB0;
  sample = func_80154F78(x, y, 3);
  if (D_1F800000[0] != 0 && (s16)sample >= 0x41) {
    if (*(s16*)(D_1F800044 + 0x3e) < (s16)func_80154F28(x, y)) {
      D_1F800044[0x2b] = 0;
    }
  }

  x = *(s32*)(D_1F800044 + 0x34) + D_80181BBC;
  y = *(s32*)(D_1F800044 + 0x38) + D_80181BC0;
  sample = func_80154F78(x, y, 5);
  if (D_1F800000[0] != 0 && (s16)sample >= 0x41) {
    if (*(s16*)(D_1F800044 + 0x3e) < (s16)func_80154F28(x, y)) {
      D_1F800044[0x2b] = 0;
    }
  }

  func_8014D8D4((u8)((D_1F800044[0x8] >> 1) + 0x42));
  D_1F800044[0x2]++;
}
