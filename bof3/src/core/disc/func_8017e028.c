#include <libapi.h>

#include "internal.h"

void InitCARD2(s32 arg0);
void _patch_card(void);
void _patch_card2(void);
void func_8017ee0c(void);
void func_8017ee1c(void);

/* does: resets PAD clearing, applies the boot-side memory card init/patch
 * sequence under the event guard, then restores the event state.
 * @source: 0x8017e028 FUN_8017e028
 */
void func_8017e028(s32 arg0) {
  ChangeClearPAD(0);
  func_8017ee0c();
  InitCARD2(arg0);
  _patch_card();
  _patch_card2();
  func_8017ee1c();
}
