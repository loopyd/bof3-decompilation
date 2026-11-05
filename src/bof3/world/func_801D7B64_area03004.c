#include "bof3/world/area03004_internal.h"

/* @source 0x801D7B64
 * @behavior draws the AREA030 entry-panel counter row at its fixed coordinates:
 * selects draw mode 0/1 and stamps sprite 0x12 at (0xF0, 0x42), submits the
 * two-character rodata label D_801D0C10 at (0x11C, 0x45) through the shared
 * text drawer func_8014FF0C, then formats the signed halfword at offset 0x3E of
 * the work record published at the scratchpad cursor 0x1F800044 plus one
 * through the "%2d" format into the shared 0x80145AD4 buffer, rewrites a
 * leading space of that buffer to 0xFF and submits the two-token run at
 * (0xFA, 0x45) through func_8014F800.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D7B64(void) {
  u8* buf;
  u8* record;

  submitTpageDrawMode(0, 1);
  func_801E0DCC(0x12, 1, 0xF0, 0x42);
  func_8014FF0C(0x11C, 0x45, 0, D_801D0C10);

  buf = D_80145AD4;
  record = D_1F800044;
  sprintf((char*)buf, (const char*)D_801D0C04, FIELD_REF(s16, record, 0x3E) + 1);
  if (buf[0] == 0x20) {
    buf[0] = 0xFF;
  }
  func_8014F800(0xFA, 0x45, 0, 2, (u32)buf);
}
