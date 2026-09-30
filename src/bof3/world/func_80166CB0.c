#include "bof3/core/slus_internal.h"

/* @behavior reads the map byte at the signed column and row offset scaled by
 * the current map row stride, relative to the current map data base pointer.
 * The original signs both arguments from their low halves, scales the row by
 * the stride byte, then adds the column.
 * @source 0x80166CB0
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_80166CB0(s16 arg0, s16 arg1) {
  return *(u8*)(D_8014931C + arg1 * D_80104000 + arg0);
}
