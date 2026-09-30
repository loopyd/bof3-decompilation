#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 panel-step initializer: func_801DDE2C first republishes the
 * two area030 record pointers selected by the level flags 0x80145026/0x80145028;
 * the scratch work record published at 0x1F800044 then has its six step words at
 * offsets 0x0C..0x20 and its countdown byte 9 cleared while the shared
 * active-record index D_801E3208 is disabled with 0xFF. The state helper
 * func_801DDE94 receives mode 6 while either level flag is clear and mode 0 when
 * both are set, after which resource 0xA is requested through func_8014D6DC
 * (second argument 2) while byte 0x79 of the record at D_80146250 is clear and
 * resource 0xB through func_8014D6B8 otherwise; finally the dispatch byte 3 of
 * the work record published at 0x1F800044 is advanced.
 * @source 0x801D9D38
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D9D38(void) {
  u8* work;
  u8* record;

  func_801DDE2C();
  record = D_1F800044;
  D_801E3208 = 0xFF;
  *(u32*)(record + 0x0C) = 0;
  *(u32*)(record + 0x10) = 0;
  *(u32*)(record + 0x14) = 0;
  *(u32*)(record + 0x18) = 0;
  *(u32*)(record + 0x1C) = 0;
  *(u32*)(record + 0x20) = 0;
  record[9] = 0;
  if (D_80145026 == 0 || D_80145028 == 0) {
    func_801DDE94(6);
  } else {
    func_801DDE94(0);
  }
  if (D_80146250[0x79] == 0) {
    func_8014D6DC(0xA, 2);
  } else {
    func_8014D6B8(0xB);
  }
  work = WORLD00_AREA030_SCRATCH_PTR;
  work[3] = (u8)(work[3] + 1u);
}
