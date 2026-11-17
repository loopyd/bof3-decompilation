#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E0770
 * @behavior Reports whether the AREA030 work record published at the
 * scratchpad cursor 0x1F800044 sits inside the shared staging window: the
 * check runs only while the 0x80143FC9 mode byte is 4, the 0x801E3208 byte is
 * 0xFF, the 0x80144199 gate byte is clear and the 0x80145E92 phase byte is 3,
 * and it then returns whether the record x at +0x34, y at +0x38 and signed
 * timer at +0x3E are within 0x7FFF, 0x7FFF and 0x400 of the shared reference
 * position 0x80143FFC / 0x80144000 / 0x80144006.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_801E0770(void) {
  Area030WorkRecord* work;
  s32 result;
  s32 dz;
  s32 sample;
  s32 test;
  s32 bound;

  result = 0;
  do {
    sample = D_80143FC9;
    test = 4;
    if (sample != test) {
      break;
    }
    sample = D_801E3208;
    test = 0xFF;
    if (sample != test) {
      break;
    }
    test = D_80144199[0];
    if (test != 0) {
      break;
    }
    sample = D_80145E92;
    test = 3;
    if (sample != test) {
      break;
    }
    do {
      bound = 0x7FFF;
    } while (0);
    work = (Area030WorkRecord*)D_1F800044;
    sample = D_80143FFC;
    test = work->x_34;
    test -= sample;
    if (test < 0) {
      test = -test;
    }
    test = bound < test;
    if (test != 0) {
      break;
    }
    test = work->y_38;
    sample = D_80144000;
    test -= sample;
    if (test < 0) {
      test = -test;
    }
    test = bound < test;
    if (test != 0) {
      break;
    }
    dz = work->counter_3E;
    test = D_80144006;
    dz -= test;
    if (dz < 0) {
      dz = -dz;
    }
    result = dz < 0x400;
  } while (0);
  return result;
}
