#include "internal.h"

/* @behavior seeds one routed setup path and enters secondary state 2 on success.
 * @source 0x801F6EB0
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
