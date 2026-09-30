#include "bof3/battle/battle03_internal.h"

/* @source 0x801E9390
 * @behavior Ages the seven per-actor countdown bytes at 0x801462FC: for each
 * index below seven, when the selected-actor byte 0x801462F4 equals that index
 * the countdown is raised by two while it is below eight, otherwise it is
 * lowered by two while it is non-zero; afterwards forwards the two signed panel
 * halfwords +4/+6 of the object at 0x80148648 to func_801D7A40.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E9390(void) {
  u8 index = 0u;
  u8* selected = &D_801462F4;

  do {
    if (*selected == index) {
      if (D_801462FC[index] < 8u) {
        D_801462FC[index] = D_801462FC[index] + 2u;
      }
    } else if (D_801462FC[index] != 0u) {
      D_801462FC[index] = D_801462FC[index] - 2u;
    }
    index += 1u;
  } while (index < 7u);

  func_801D7A40(*(s16*)&D_80148648[4], *(s16*)&D_80148648[6]);
}
