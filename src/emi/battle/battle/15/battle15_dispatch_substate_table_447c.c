#include "internal.h"

/*
 * @source 0x80099328
 * @behavior Dispatches the selection-phase handler indexed by D_801462E4.
 */
void battle15_dispatch_substate_table_447c(void) {
  D_800B447C[D_801462E4]();
}
