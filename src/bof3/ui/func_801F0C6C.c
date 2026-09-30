#include "bof3/ui/commu00_internal.h"

/* @source 0x801F0C6C
 * @behavior marks the scratch task active and stores the indexed task slot's
 * label word: the record-kind slot level byte plus 0xC8, or the fixed 0xCC
 * label when the village-expansion state bytes select it.
 * @status exact
 * @match 100.00
 * @residual none
 * Live asm-diff 52/52 (100.00%) instructions, 208->208 bytes, byte-match rc=0;
 * independent review pending. The 0xCC label needs one shared statement (as
 * written below): an `else if` chain with a second 0xCC arm compiles to a
 * duplicate arm block (+7 instructions, +0x1C bytes) instead of reusing the
 * single store block.
 */
void func_801F0C6C(u8 task_index, u8 record_kind_index) {
  commu00ScratchTask->active = 1;

  if ((D_801455C4 == 7 && D_801457A9[(record_kind_index & 0xFF) * 8] != 0) ||
      (D_801455C3 == 0xA &&
       D_801457A9[(record_kind_index & 0xFF) * 8] == 0)) {
    taskLabelWords[(task_index & 0xFF) * 76] = 0xCC;
  } else {
    taskLabelWords[(task_index & 0xFF) * 76] =
        D_801457A9[(record_kind_index & 0xFF) * 8] + 0xC8;
  }
}
