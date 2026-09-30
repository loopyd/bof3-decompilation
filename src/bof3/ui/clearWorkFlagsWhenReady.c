#include "bof3/ui/game00_internal.h"

/* File-local prototype: the target header deliberately does not declare this
 * shared main-executable service (see the note there - its callers disagree on
 * the signature, and a header declaration collides with
 * src/bof3/ui/func_801A0514.c's struct GameWorkArea* prototype). */
void func_8014D290(void);

/*
 * @source 0x8019C910
 * @behavior Runs the shared readiness helper func_8014DAEC and, when it reports
 * ready (non-zero byte), clears the scratchpad work-area flags through
 * clearWorkFlags; while the helper is not ready the shared main-executable
 * service func_8014D290 runs instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkFlagsWhenReady(void)
{
  if (func_8014DAEC() != 0u) {
    clearWorkFlags();
  } else {
    func_8014D290();
  }
}
