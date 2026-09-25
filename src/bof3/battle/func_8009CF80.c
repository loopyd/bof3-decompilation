#include "bof3/battle/battle15_internal.h"

/* @source 0x8009CF80
 * @behavior Stores value into the selection byte selected by index: 0xCF..0xD5
 *   for indexes 0..6 and 0xD7 for index 7; indexes of 8 or more store nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009CF80(u8 *obj, u8 index, u8 value) {
  switch (index) {
  case 0:
    obj[0xCF] = value;
    break;
  case 1:
    obj[0xD0] = value;
    break;
  case 2:
    obj[0xD1] = value;
    break;
  case 3:
    obj[0xD2] = value;
    break;
  case 4:
    obj[0xD3] = value;
    break;
  case 5:
    obj[0xD4] = value;
    break;
  case 6:
    obj[0xD5] = value;
    break;
  case 7:
    obj[0xD7] = value;
    break;
  default:
    break;
  }
}
