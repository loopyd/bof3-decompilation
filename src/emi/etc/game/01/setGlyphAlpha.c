#include "internal.h"

/* @behavior writes one alpha value to the three color channels of a frontend
 * primitive.
 * @source 0x801D18E8
 */
void setGlyphAlpha(u8* primitive, u8 alpha) {
  primitive[4] = alpha;
  primitive[5] = alpha;
  primitive[6] = alpha;
}
