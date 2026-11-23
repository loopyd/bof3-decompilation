#include "bof3/world/area03004_internal.h"

/* @behavior issues the fixed five-argument panel request 20, 18, 280, 19, 0.
 * @source 0x801D3908
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderFixedPanelRectangle(void) {
  renderFilledInsetBorder(20, 18, 280, 19, 0);
}
