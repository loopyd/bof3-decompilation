#include "bof3/world/area03004_internal.h"

/* @behavior When countdown byte 9 is zero on entry, select a four-byte motion step using
 * byte 10, redirect a -1 marker, request its low seven bits, reload byte 9 and
 * advance byte 10. Otherwise decrement byte 9 and conditionally step the pair
 * sequence. Integrate the preceding step's signed deltas, subtract its byte-2
 * value from work byte 6 modulo 256, and draw the marker row. Decrementing to
 * zero does not select a new step in the same call.
 * @source 0x801D6CB0
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The published motionWork cell 0x1F800044 is volatile, so every access group
 * re-reads it: the entry read, the two post-call reads of the expired-countdown
 * branch, and the integration read. Each group keeps its own local for the read
 * it belongs to. That is what the original does too, and it is also what fixes
 * the register assignment: a shared local for the two post-call groups puts them
 * in $a0 and shifts both the entry read and the integration read to $a1, while
 * separate locals reproduce the original's $a0 entry/tail and $v1 post-call
 * registers (532/532 bytes, 133/133 instructions).
 */
void stepPanelMotionScriptAndDrawMarkerRowWorld00Area03004(void) {
  u8*  work;
  s32  stepIndex;
  u8*  motionWork;
  u8*  stepWork;

  work = D_1F800044;
  if (work[9] == 0) {
    stepIndex = work[10];
    if (D_801E207C[stepIndex].unk_00 == -1) {
      work[10] = D_801E207C[stepIndex].count_01;
    }
    func_8014D8D4(((Area030StepRecordBytes*)D_801E207C)[D_1F800044[10]].unk_00 &
                  0x7F);
    stepWork = D_1F800044;
    stepWork[9] = D_801E207C[stepWork[10]].count_01;
    stepWork = D_1F800044;
    stepWork[10] = stepWork[10] + 1;
  } else {
    work[9] = work[9] - 1;
    if ((D_801E207C[D_1F800044[10] - 1].unk_00 & 0x80) == 0) {
      func_8014D978();
    }
  }

  motionWork = D_1F800044;
  *(s32*)(motionWork + 0x0C) =
      *(s32*)(motionWork + 0x0C) + D_801E207C[motionWork[10] - 1].unk_02;
  *(s32*)(motionWork + 0x10) =
      *(s32*)(motionWork + 0x10) + D_801E207C[motionWork[10] - 1].unk_03;
  motionWork[6] = motionWork[6] - (u8)D_801E207C[motionWork[10] - 1].unk_02;
  if (D_801E207C[D_1F800044[10] - 1].unk_03 < 0) {
    clampMarkerPositionsAndDrawPanelRowWorld00Area03004(0);
  } else {
    clampMarkerPositionsAndDrawPanelRowWorld00Area03004(1);
  }
  drawPanelCellAndOptionalBlinkingHighlightWorld00Area03004(0xCE, 0x5C, 0x20, 0, 1, 0);
  drawMirroredPanelCellQuadWorld00Area03004_801D2C34(0xE3, 0x69, 0, 1);
}
