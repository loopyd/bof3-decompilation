#include "internal.h"

/* @behavior advances the world-front state machine, coordinating selection
 * transitions, front effects, and shared scenario updates.
 * @source 0x80197378 func_80197378
 */
void func_80197378(void) {
  u32* counter;

  func_801a782c();
  func_801991b8();

  switch (DAT_80143bb0) {
    case 0:
      break;
    case 1:
      func_8014ecac(2);
      func_80198bc4(0);
      DAT_80143b90 = 3;
      DAT_80143b92 = 0;
      break;
    case 2:
      DAT_80143b90 = 4;
      break;
    case 3:
      DAT_801462e0 = 0;
      DAT_801462e1 = 0;
      DAT_801462e2 = 0;
      DAT_80145e9b = 0;
      DAT_80145fdb = 0;
      DAT_8014611b = 0;
      counter = &DAT_8014502c;
      DAT_80143b90 = 5;
      DAT_80143b92 = 0;
      (*counter)++;
      break;
    case 4:
      DAT_80143b90 = 6;
      break;
    case 5: {
      u8 selection;

      DAT_8014625a |= 0x40;
      selection = DAT_80145029;
      if (selection != 0xff && DAT_80143f1f != selection) {
        func_8015d4f8(DAT_80181eba[selection * 4], DAT_80181ebb[selection * 4],
                      100, 16);
      }
      if (DAT_80143f1e != 0xff) {
        func_8014ecac(DAT_80143f1e);
        func_80198bc4(0);
      }
      selection = DAT_80145029;
      if (selection != 0xff && DAT_80143f1f != selection) {
        func_8015d404(DAT_80181eba[selection * 4], DAT_80181ebb[selection * 4]);
      }
      DAT_80143b90 = 1;
      break;
    }
    case 6:
      DAT_80148650 = 6;
      DAT_8014865c = -1;
      DAT_80148651 = 0;
      DAT_80148652 = 0;
      DAT_80143b90 = 7;
      DAT_80143b92 = 0;
      break;
    case 7:
      DAT_80143b90 = 9;
      DAT_80143b92 = 0;
      DAT_80148650 = 0;
      DAT_80148651 = 0;
      DAT_80148652 = 0;
      break;
    case 8:
      DAT_80143b90 = 10;
      DAT_80143b92 = 0;
      DAT_80148650 = 0;
      DAT_80148651 = 0;
      DAT_80148652 = 0;
      break;
    case 9:
      DAT_8014626c = 1;
      DAT_8014626d = 0;
      DAT_8014626e = 0;
      DAT_8014626f = 0;
      DAT_80146270 = 0;
      DAT_80143b90 = 11;
      break;
  }
}
