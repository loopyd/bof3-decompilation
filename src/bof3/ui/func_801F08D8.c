#include "bof3/ui/commu00_internal.h"

/* @source 0x801F08D8
 * @behavior Configures the indexed commu00 task slot as a variant-6 task and
 *           dispatches it: publishes the slot as the scratchpad task cursor,
 *           records the source index, derives the stride-8 active-record index
 *           from the source-indexed record byte, clears the slot lead bytes,
 *           seeds the slot coordinate pair and resource id from the
 *           variant-rotation record selected by the record index and the
 *           rotation byte, seeds the slot table entry selected by the source
 *           index, selects the slot label through func_8015477C, runs the
 *           record-kind handler selected by the stride-8 kind byte, and
 *           advances that record's rotation byte.
 * @status partial
 * @match 78.00
 * @residual Live asm-diff 156/200 instructions (78.00%), 800 vs 796 bytes, first
 * difference +0x0000: the compiler switch table cannot be placed at the
 * original 0x801EEC0C (4 mod 8) because the reviewed section-placement route
 * links the emitted 8-aligned .rodata input through `ld --section-start` and
 * pads it by four bytes; declaring that placement makes asm-diff/byte-match
 * abort with "reviewed .rodata placement ... does not match original bytes"
 * and no diff, so it is intentionally undeclared. The remaining callee-saved
 * allocation/scheduling permutation in the slot-seeding head (frame 64 versus
 * 48; one extra pointer reload) is retained as measured. Next untried evidence:
 * a harness route that can place an 8-aligned jump-table .rodata input at a
 * 4-mod-8 address; not a clean-C lever.
 */
void func_801F08D8(u8 source_index, u8 task_index) {
  u8 record_index;
  u8 record_kind;

  record_index = (u8)(D_801455C9[(source_index & 0xFF) * 8] - 1);

  commu00ScratchTask = &D_80146888[task_index];
  commu00ScratchTask->source_index = source_index;
  commu00ScratchTask->mode = 6;

  D_80146888[task_index].unk_49[0x2F] = 0;
  commu00ScratchTask->state = 0;

  commu00ScratchTask->field_34 = 0;
  commu00ScratchTask->field_36 =
      D_801F25A4[record_index * 9 + variantRotation[record_index] * 3];
  commu00ScratchTask->field_38 = 0;
  commu00ScratchTask->field_3a =
      D_801F25A4[record_index * 9 + variantRotation[record_index] * 3 + 1];
  /* Both coordinate words are read back as one 32-bit pair, matching the
   * original's packed coordinate read: the low halfword is cleared and the
   * high halfword carries the variant byte. */
  commu00ScratchTask->field_3e =
      func_8015477C(*(s32 *)&commu00ScratchTask->field_34,
                    *(s32 *)&commu00ScratchTask->field_38);
  commu00ScratchTask->resource_id =
      D_801F25A4[record_index * 9 + variantRotation[record_index] * 3 + 2];
  commu00ScratchTask->reset_flag = 0;

  func_8014DD3C(D_801F24FC[commu00ScratchTask->source_index % 6]);
  func_8015BAC4(commu00ScratchTask->resource_id);

  record_kind = D_801457A8[record_index * 8];

  switch (record_kind) {
  case 0:
    activateTaskWithRandomLabel(task_index);
    break;
  case 4:
    func_801F0C6C(task_index, record_index);
    break;
  case 5:
    func_801F0D3C(task_index, record_index);
    break;
  case 6:
    func_801F0E1C(task_index, record_index);
    break;
  case 7:
    activateTaskStatusC003(task_index);
    break;
  case 8:
    activateScratchTaskWithBandLabel(source_index, task_index, record_index);
    break;
  case 9:
    func_801F0FBC(source_index, task_index, record_index);
    break;
  case 10:
    func_801F1064(task_index, record_index);
    break;
  case 11:
    func_801F1110(task_index, record_index);
    break;
  case 12:
    setVariantTaskStatus(task_index, record_index);
    break;
  case 13:
    activateTaskStatusC00A(task_index);
    break;
  }

  variantRotation[record_index] += 1;
}
