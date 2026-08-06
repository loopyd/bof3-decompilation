#include "internal.h"

/* @source 0x801E4760
 * @behavior loads the non-volatile scratchpad pointer cell at 0x1F800044, then invokes its byte-3-selected handler from the local 0x801EB444 dispatch table
 */
void dispatchByte3TableB444(void) {
  D_801EB444[battleWork[3]]();
}
