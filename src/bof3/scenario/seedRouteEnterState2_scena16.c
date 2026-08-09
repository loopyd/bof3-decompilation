#include "bof3/scenario/scena16_internal.h"

/* @behavior seeds one routed setup path and enters secondary state 2 on success.
 * @source 0x801F6EB0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void seedRouteEnterState2(void) {
  if (func_8015B5D4(D_8014686C, 0) == 0) {
    D_8014832E = 0u;
    func_8015B580(D_8014686C, 0);
    func_8015C088();
    D_80146874 = 2;
  }

  if (D_80143F03 == 2) {
    D_8014832E = 0x1fu;
    func_8015C100();
  }
}
