#include "bof3/scenario/scena16_internal.h"

/* @behavior seeds one routed setup path and enters secondary state 3 on success.
 * It arms the two shared halfword channels D_801492D8/D_8014932C with 0x100, clears
 * D_801492DC and D_801492DA, runs func_8015C100, and, when
 * func_8015B5D4(D_8014686C, 1) reports success as zero, clears the effect bank
 * D_8014832E, runs func_8015B580(D_8014686C, 1) and enters secondary state 3 by
 * storing 3 in D_80146874. D_8014686C is read once per call, and the second call's
 * constant argument is issued in its delay slot.
 * @source 0x801F6E30
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedRouteEnterState3(void) {
  D_801492D8 = 0x100u;
  D_801492DC = 0u;
  D_801492DA = 0u;
  D_8014932C = 0x100u;
  func_8015C100();

  if (func_8015B5D4(D_8014686C, 1) == 0) {
    D_8014832E = 0u;
    func_8015B580(D_8014686C, 1);
    D_80146874 = 3;
  }
}
