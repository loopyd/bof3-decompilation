#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 selection-panel step advancer on the work record published
 * at the scratchpad cursor 0x1F800044: while its countdown byte 9 is nonzero it
 * decrements that byte and, unless bit 7 of the current step record's first
 * byte is set, runs the shared func_8014D978 helper. Once the countdown expires
 * it looks up the step record at 0x801E207C through the record's step byte 10,
 * re-seeds byte 10 from the record's count byte when the record starts with the
 * 0xFF marker, requests the record's low 7 bits through func_8014D8D4, arms the
 * countdown byte 9 from the record's count byte and advances the step byte 10.
 * It then integrates the stepped-from record: word 0x0C and word 0x10 gain the
 * record's signed deltas and byte 6 loses its second delta, the sign of the
 * record's fourth byte selects the func_801D6B28 marker mode, and the panel row
 * is redrawn through func_801D3244 (0xCE, 0x5C, 0x20, 0, 1, 0) and
 * func_801D2C34 (0xE3, 0x69, 0, 1).
 * @source 0x801D6CB0
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The published cursor cell 0x1F800044 is volatile, so every access group
 * re-reads it: the entry read, the two post-call reads of the expired-countdown
 * branch, and the integration read. Each group keeps its own local for the read
 * it belongs to. That is what the original does too, and it is also what fixes
 * the register assignment: a shared local for the two post-call groups puts them
 * in $a0 and shifts both the entry read and the integration read to $a1, while
 * separate locals reproduce the original's $a0 entry/tail and $v1 post-call
 * registers (532/532 bytes, 133/133 instructions).
 */
void func_801D6CB0(void) {
  u8*  work;
  s32  step;
  u8*  cursor;
  u8*  refresh;

  work = D_1F800044;
  if (work[9] == 0) {
    step = work[10];
    if (D_801E207C[step].unk_00 == -1) {
      work[10] = D_801E207C[step].count_01;
    }
    func_8014D8D4(((Area030StepRecordBytes*)D_801E207C)[D_1F800044[10]].unk_00 &
                  0x7F);
    refresh = D_1F800044;
    refresh[9] = D_801E207C[refresh[10]].count_01;
    refresh = D_1F800044;
    refresh[10] = refresh[10] + 1;
  } else {
    work[9] = work[9] - 1;
    if ((D_801E207C[D_1F800044[10] - 1].unk_00 & 0x80) == 0) {
      func_8014D978();
    }
  }

  cursor = D_1F800044;
  *(s32*)(cursor + 0x0C) =
      *(s32*)(cursor + 0x0C) + D_801E207C[cursor[10] - 1].unk_02;
  *(s32*)(cursor + 0x10) =
      *(s32*)(cursor + 0x10) + D_801E207C[cursor[10] - 1].unk_03;
  cursor[6] = cursor[6] - (u8)D_801E207C[cursor[10] - 1].unk_02;
  if (D_801E207C[D_1F800044[10] - 1].unk_03 < 0) {
    func_801D6B28(0);
  } else {
    func_801D6B28(1);
  }
  func_801D3244(0xCE, 0x5C, 0x20, 0, 1, 0);
  func_801D2C34(0xE3, 0x69, 0, 1);
}
