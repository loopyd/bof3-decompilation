#include "bof3/context.h"
#include "internal.h"

extern void InitCARD2(s32 arg0);
extern void _patch_card(void);
extern void _patch_card2(void);

/* @behavior resets PAD clearing, applies the boot-side memory card init/patch
 * sequence under the event guard, then restores the event state.
 * @source 0x8017E028
 */
void func_8017E028(s32 arg0) {
  ChangeClearPAD(0);
  EnterCriticalSection();
  InitCARD2(arg0);
  _patch_card();
  _patch_card2();
  ExitCriticalSection();
}
