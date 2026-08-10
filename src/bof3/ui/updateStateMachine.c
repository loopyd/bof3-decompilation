#include "bof3/ui/game00_internal.h"

/* @behavior advances the world-front state machine, coordinating selection
 * transitions, front effects, and shared scenario updates.
 * @source 0x80197378
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void updateStateMachine(void) {
  u32* counter;

  dispatchScenarioHandlerAndState();
  func_801991B8();

  switch (D_80143BB0) {
    case 0:
      break;
    case 1:
      func_8014ECAC(2);
      waitTransition(0);
      D_80143B90 = 3;
      D_80143B92 = 0;
      break;
    case 2:
      D_80143B90 = 4;
      break;
    case 3:
      D_801462E0 = 0;
      D_801462E1 = 0;
      D_801462E2 = 0;
      D_80145E9B = 0;
      D_80145FDB = 0;
      D_8014611B = 0;
      counter = &D_8014502C;
      D_80143B90 = 5;
      D_80143B92 = 0;
      (*counter)++;
      break;
    case 4:
      D_80143B90 = 6;
      break;
    case 5: {
      u8 selection;

      D_8014625A |= 0x40;
      selection = frontSelection;
      if (selection != 0xff && D_80143F1F != selection) {
        startSelectionFx(D_80181EBA[selection * 4], D_80181EBB[selection * 4], 100,
                      16);
      }
      if (D_80143F1E != 0xff) {
        func_8014ECAC(D_80143F1E);
        waitTransition(0);
      }
      selection = frontSelection;
      if (selection != 0xff && D_80143F1F != selection) {
        stopSelectionFx(D_80181EBA[selection * 4], D_80181EBB[selection * 4]);
      }
      D_80143B90 = 1;
      break;
    }
    case 6:
      D_80148650 = 6;
      D_8014865C = -1;
      D_80148651 = 0;
      D_80148652 = 0;
      D_80143B90 = 7;
      D_80143B92 = 0;
      break;
    case 7:
      D_80143B90 = 9;
      D_80143B92 = 0;
      D_80148650 = 0;
      D_80148651 = 0;
      D_80148652 = 0;
      break;
    case 8:
      D_80143B90 = 10;
      D_80143B92 = 0;
      D_80148650 = 0;
      D_80148651 = 0;
      D_80148652 = 0;
      break;
    case 9:
      D_8014626C = 1;
      D_8014626D = 0;
      D_8014626E = 0;
      D_8014626F = 0;
      D_80146270 = 0;
      D_80143B90 = 11;
      break;
  }
}
