#include "bof3/context.h"
#include "internal.h"

/* PsyQ LIBAPI dequeue of an interrupt registration point; absent from the
 * vendored libapi.h. */
extern void SysDeqIntRP(int pri, void* rp);

/* @behavior dequeues priority-1 interrupt registration point D_8018DB40 under
 * a critical section and returns 1.
 * @source 0x8017F27C
 */
int func_8017F27C(void) {
  EnterCriticalSection();
  SysDeqIntRP(1, D_8018DB40);
  ExitCriticalSection();
  return 1;
}
