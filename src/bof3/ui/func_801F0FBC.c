#include "bof3/ui/commu00_internal.h"

/* @source 0x801F0FBC
 * @behavior reads the state byte of the source-indexed active record and either
 *           stores a state-selected label word on the indexed task slot, or
 *           clears the scratch task's lead byte for state 1.
 * @status exact
 * @match 100.00
 * @residual none
 * Live asm-diff 42/42 instructions (100.00%) and byte-match rc=0; independent
 * review pending. The record state byte is read non-volatilely through the
 * byte-addressed stride-8 record view; the same byte read through the volatile
 * record struct forces a separate zero-extension instruction.
 */
void func_801F0FBC(u8 source_index, u8 task_index, u8 record_kind_index) {
  u8 state;

  /* Record view: stride-8 state byte following the record's kind byte. */
  state = D_801455C9[(source_index & 0xFF) * 8 + 1];

  if (state == 0) {
    commu00ScratchTask->active = 1;
    taskLabelWords[(task_index & 0xFF) * 76] = 0x51;
  } else if (state == 1) {
    commu00ScratchTask->unk_00 = 0;
  } else {
    commu00ScratchTask->active = 1;
    taskLabelWords[(task_index & 0xFF) * 76] = 0xC004;
  }
}
