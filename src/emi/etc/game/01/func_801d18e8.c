#include "internal.h"

/* @behavior writes one alpha value to the three color channels of a frontend
 * primitive.
 * @source 0x801d18e8 func_801d18e8
 */
void func_801d18e8(u8* primitive, u8 alpha) {
  primitive[4] = alpha;
  primitive[5] = alpha;
  primitive[6] = alpha;
}
