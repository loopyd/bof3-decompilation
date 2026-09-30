#include "bof3/ui/game00_internal.h"

/* @behavior Queries the shared route-parameterised position service
 *           func_80154F78 with the two incoming coordinates and the scratch
 *           work-area route index, then reports whether the returned value
 *           shifted up by 16 bits exceeds 0x400000; the report is suppressed to
 *           zero while the scratchpad byte at 0x1F800000 is zero.
 * @source 0x801BB760
 * @status partial
 * @match 65.00
 * @residual Two register roles differ; everything else (block shape, branch
 *           direction, delay-slot content, 80-byte frame) matches. The original
 *           copies the func_80154F78 result out of the return register into $a0
 *           (`move a0,v0` at +0x1C) and keeps the compared result in $v0; this
 *           spelling leaves the call result in $v0 and allocates the result
 *           pseudo to $a0, so the entry copy is missing and the epilogue gains
 *           `move v0,a0` (19 insns / 76 bytes vs 20 / 80). First difference: the
 *           absent `move a0,v0`. One four-variant `lift sweep` (in-process route)
 *           measured the two-return spellings higher at 17/20, 80 bytes, but they
 *           invert the branch (`bne` into the compare block plus a spare `j`)
 *           against the original's `beqz` over it, so this topologically faithful
 *           spelling is retained as the frontier candidate. Next untried rung:
 *           clean-C temporary/lifetime levers that move the result pseudo into
 *           $v0 without register pins.
 */
s32 func_801BB760(s32 arg0, s32 arg1) {
  s32 result;
  s32 value;

  value = func_80154F78(arg0, arg1, g_game_work->route_index_08);
  result = 0;
  if (D_1F800000[0] != 0) {
    result = (value << 16) > 0x400000;
  }
  return result;
}
