#include "bof3/world/area03004_internal.h"

/* @behavior invokes the AREA030 companion dispatch entry.
 * @source 0x801E0C20
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchCompanion(void) {
  dispatchArea030CompanionHandler();
}
