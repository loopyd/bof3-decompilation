#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 work-record step handler: calls the shared helper
 * func_8014D978; then, when the halfword at offset 0x58 of the work record
 * published at the scratchpad cursor 0x1F800044 equals 6, increments the shared
 * byte counter D_80143FC9, clears record bytes 0x0B and 0x07 and stores 3 in
 * record byte 0x03.
 * @source 0x801DA714
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DA714(void) {
  u8* work;

  func_8014D978();
  work = D_1F800044;
  if (*(u16*)(work + 0x58) == 6) {
    u8* counter = &D_80143FC9;

    *counter = (u8)(*counter + 1u);
    work[0x0B] = 0;
    D_1F800044[0x07] = 0;
    D_1F800044[0x03] = 3;
  }
}
