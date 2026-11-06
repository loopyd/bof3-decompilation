#include "bof3/world/area03004_internal.h"

/* @behavior queues the two AREA030 icon strip primitives for the selected mode.
 * @source 0x801D2034
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The fourth argument is unsigned: the callers narrow the icon variant into
 * arg3 with `andi 0xff` (0x801D66DC) and the AREA030 name step at 0x801D1F38
 * converts its clamped 0..4 value the same way, which a signed parameter would
 * emit as a sign extension instead. The body's own bytes are identical either
 * way, because it truncates the sprite index with an explicit cast. */
void queueIconStrip(s16 arg0, s16 arg1, u8 arg2, u8 arg3) {
  submitTpageDrawMode(0, arg2);
  func_801E0DCC(0x17, arg2, arg0, arg1);
  func_801E0DCC((u8)(arg3 + 0x17), arg2, (s16)(arg0 + 0x28), arg1);
}
