#include "bof3/world/area03004_internal.h"

/* @source 0x801DC5D4
 * @behavior AREA030 work-record dispatch step, handler 1 of the local table at
 * 0x801E22B0 selected by byte 3 of the scratch work record published at
 * 0x1F800044: while the shared selection gate byte D_80144199_BYTE holds 3, it
 * calls the frontend effect helper func_8015D404 with (0, 1), queues the
 * frontend cue func_80161C20(0x28, 100, 8), dispatches sound cue 0x203 through
 * func_8015DF18, requests state 4 from the target-local state helper
 * func_801DDE94 and advances that record's dispatch byte 3; the shared helper
 * func_8014D978 runs unconditionally afterwards and no result is used here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DC5D4(void) {
  u8* work;

  if (D_80144199_BYTE == 3) {
    func_8015D404(0, 1);
    func_80161C20(0x28, 100, 8);
    func_8015DF18(0x203);
    func_801DDE94(4);
    work = D_1F800044;
    work[3]++;
  }
  func_8014D978();
}
