#include "internal.h"

/* @source 0x801E27BC
 * @behavior handler-table dispatcher: calls D_801E5D68[g_PanelTaskRoot->unk_00[2]],
 *           indexing the handler table with panel task byte 2 (framed jalr,
 *           no sibling call).
 */
void NO_SIBLING_CALLS func_801E27BC(void) {
  D_801E5D68[g_PanelTaskRoot->unk_00[2]]();
}
