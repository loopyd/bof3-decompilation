#include "bof3/world/area03004_internal.h"

/* @source 0x801E0680
 * @behavior AREA030 work-record entry gate: while the shared 0x80143FC9 mode
 * byte is 4, the 0x801E3208 byte is 0xFF and the 0x80145E92 phase byte is 3 it
 * reports a match without arming the work record published at the scratchpad
 * cursor 0x1F800044 - either because that record byte 0x06 is 0x16 while the
 * low nibble of the 0x801E3210 byte 0x0E equals the mode byte, or because
 * func_801BDD58 reports one for the record x at +0x34, y at +0x38 and signed
 * timer at +0x3E with the range (0x80143FCF - 0x801E3210[0x0F] + 3) and the
 * entry-table pointer 0x80143FC8; otherwise, and whenever one of the three
 * gates fails, it stores 1 in the record byte 0x01 and reports no match.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_801E0680(void) {
  Area030WorkRecord* work;
  u8* limit;
  u8* entry;
  u8* table;
  u8 limit_value;
  s32 ref;

  if (D_80143FC9 == 4 && D_801E3208 == 0xFF && D_80145E92 == 3) {
    if (D_1F800044[6] == 0x16 && (D_801E3210[0x0E] & 0xF) == D_80143FC9) {
      return 1;
    }
    table = D_801E3210;
    limit = (u8*)&D_80143FCF;
    limit_value = *limit;
    /* The table byte enters the range difference as a promoted word value. */
    ref = table[0x0F];
    entry = limit - 7;
    work = (Area030WorkRecord*)D_1F800044;
    if (func_801BDD58(work->x_34, work->y_38, work->counter_3E,
                      (u8)(limit_value - ref + 3),
                      entry) != 0) {
      return 1;
    }
  }
  D_1F800044[1] = 1;
  return 0;
}
