#include "internal.h"

/* @source 0x801E1C58
 * @behavior sets two bytes in the current scratchpad work record to 2 and 0.
 */
void setWorkBytes20(void) {
  D_1F800044->unk_01 = 2;
  D_1F800044->unk_02 = 0;
}
