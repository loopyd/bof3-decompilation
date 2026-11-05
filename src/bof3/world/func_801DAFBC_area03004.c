#include "bof3/world/area03004_internal.h"

/* @source 0x801DAFBC
 * @behavior AREA030 work-record dispatch step, handler 1 of the local table at
 * 0x801E2268 selected by byte 3 of the scratch work record published at
 * 0x1F800044: while the shared selection gate byte D_80144199_BYTE holds 3, it
 * calls the same-binary frontend effect helper func_8015D404 with (0, 0),
 * queues the frontend cue func_80161C20(0x29, 100, 8), requests state 2 from
 * the target-local state helper func_801DDE94, advances the shared
 * selection-step byte D_8014420D, clears byte 9 of the work record and
 * advances that record's dispatch byte 3; the shared helper func_8014D978 runs
 * unconditionally afterwards and no result is used here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DAFBC(void) {
  u8* work;

  if (D_80144199_BYTE == 3) {
    func_8015D404(0, 0);
    func_80161C20(0x29, 100, 8);
    func_801DDE94(2);
    D_8014420D++;
    work = D_1F800044;
    work[9] = 0;
    work = D_1F800044;
    work[3]++;
  }
  func_8014D978();
}
