#include "internal.h"

/* @source 0x801E57C8
 * @behavior Sets frontend mode 4 and increments the volatile global battle byte when state bit 0 is set and gate 0xBD matches.
 */
void enterFrontendMode4(void) {
    if ((D_80146328 & 1) != 0 && D_80143F04 == 0xBD) {
        func_8014ECAC(4);
        BATTLE_GLOBAL_BYTE_62E2 = BATTLE_GLOBAL_BYTE_62E2 + 1;
    }
}
