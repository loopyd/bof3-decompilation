#include "bof3/bof3.h"

void func_8015B580(u32 word, s32 bit_index);

/* @source 0x801F9D88
 * @behavior The word at 0x801FA224: the single pointer of the record-callback
 * table that the record dispatcher func_801F9D48 reads from that base with the
 * record's byte 0x7A, and calls as callback(record, D_8014686C). This callback
 * ignores the record it is handed, passes the word it is handed on to the
 * shared flag-bit helper func_8015B580 as its owner word together with bit
 * index 0, and then reports zero to the dispatcher. Takes two arguments and
 * returns the s32 zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 setFlagBit0RecordCallbackScenarioScena0400_801F9D88(void* record,
                                                         u32 flag_word) {
  func_8015B580(flag_word, 0);
  return 0;
}
