#include "internal.h"

typedef struct World00Area008State {
  u8 unk_00;
  u8 mode;
} World00Area008State;

extern void func_8014D6B8(u32 flag);

/* @behavior Selects disabled mode when the shared high-bit flag is set;
 * otherwise installs and enables the area resource, then selects mode 2.
 * @source 0x801F2C88
 */
void func_801F2C88(void) {
  volatile World00Area008State* previous;

  if ((WORLD00_AREA008_D_80146867 & 0x80u) != 0u) {
    (*(volatile World00Area008State**)0x1f800044)->mode = 9;
    return;
  }

  previous = *(volatile World00Area008State**)0x1f800044;
  *(volatile World00Area008State**)0x80146250 =
      (World00Area008State*)0x80145fd0;
  *(volatile World00Area008State**)0x1f800044 =
      (World00Area008State*)0x80145fd0;
  REG8(0x801460e8) |= 0x40;
  func_8014D6B8(0x10);
  REG8(0x80146866) = 1;
  *(volatile World00Area008State**)0x1f800044 = (World00Area008State*)previous;
  previous->mode = 2;
}
