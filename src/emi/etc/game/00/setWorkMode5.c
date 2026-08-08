#include "internal.h"

/* @behavior stores mode 5 when the scenario ID is not 2.
 * @source 0x801AD9C4
 */
void setWorkMode5(void) {
    if (D_80143BB0 == 2) {
        return;
    }

    g_game_work->field_04 = 5;
}
