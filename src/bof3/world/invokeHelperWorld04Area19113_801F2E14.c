#include "bof3/bof3.h"

void renderFullscreenColorTile(void);

/* @source 0x801F2E14
 * @behavior Thin overlay wrapper: calls the helper at 0x801F2E34 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWorld04Area19113_801F2E14(void) {
  renderFullscreenColorTile();
}
