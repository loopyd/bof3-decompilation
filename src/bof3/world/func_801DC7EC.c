#include "bof3/world/area03004_internal.h"

/* @source 0x801DC7EC */
/* @behavior Initializes three scratch work-record bytes when the shared mode is zero.
 * @status partial
 * @match 89.47
 * @residual Same-size scheduling residual: original places li v0,1 in the
 * bnez delay slot; current places it after the scratch pointer load; canonical
 * and installed historical compiler profiles produced no exact result.
 */
void func_801DC7EC(void)
{
  if (D_80143C40 == 0) {
    D_1F800044[2] = 1;
    D_1F800044[3] = 0;
    D_1F800044[4] = 0;
  }
}
