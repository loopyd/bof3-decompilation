#include "bof3/ui/shop00_internal.h"

/* @source 0x801D0EA0
 * @behavior shop UI state dispatcher: tail-calls the handler
 *           D_801E5144[D_80148650], indexing this EMI's shop handler table with
 *           the current state byte. The byte load and its scaled index precede
 *           the frame reservation and the table word is fetched into the scratch
 *           register the jalr uses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchShopUiState(void) {
  D_801E5144[D_80148650]();
}
