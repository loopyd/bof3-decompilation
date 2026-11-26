#include "bof3/ui/shop00_internal.h"

/* @source 0x801E27BC
 * @behavior handler-table dispatcher: calls D_801E5D68[g_PanelTaskRoot->unk_00[2]],
 *           indexing the handler table with panel task byte 2 (framed jalr,
 *           no sibling call).
 * @status exact
 * @match 100.00
 * @residual none
 */
void NO_SIBLING_CALLS callPanelByteTable(void) {
  D_801E5D68[g_PanelTaskRoot->unk_00[2]]();
}
