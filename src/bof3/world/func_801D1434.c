#include "bof3/world/area03004_internal.h"

/* @behavior fades the three AREA030 work-record tint bytes at offsets 0x5D..0x5F
 * down by 16 and, once the first reaches -128, clears all three, clears bit 5 of
 * work byte 0, copies byte 0x0B into byte 0x27 and advances the stage byte 3;
 * before that point it runs the target-local
 * publishPanelTemplateAndSelectStateWorld00Area03004_801D11C0 step instead.
 * @source 0x801D1434
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D1434(void) {
  u8* first;
  u8* second;
  u8* third;
  u8* cursor;
  u8* flag;

  first = (u8*)D_1F800044;
  first[0x5Du] = (u8)(first[0x5Du] - 16u);
  second = (u8*)D_1F800044;
  second[0x5Eu] = (u8)(second[0x5Eu] - 16u);
  third = (u8*)D_1F800044;
  third[0x5Fu] = (u8)(third[0x5Fu] - 16u);

  cursor = (u8*)D_1F800044;
  if ((s8)cursor[0x5Du] == -128) {
    cursor[0x5Du] = 0u;
    ((u8*)D_1F800044)[0x5Eu] = 0u;
    ((u8*)D_1F800044)[0x5Fu] = 0u;
    flag = (u8*)D_1F800044;
    flag[0u] = (u8)(flag[0u] & 0xDFu);
    third = (u8*)D_1F800044;
    third[0x27u] = third[0x0Bu];
    third = (u8*)D_1F800044;
    third[3u] = (u8)(third[3u] + 1u);
  } else {
    publishPanelTemplateAndSelectStateWorld00Area03004_801D11C0();
  }
}
