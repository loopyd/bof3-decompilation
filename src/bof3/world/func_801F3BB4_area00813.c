#include "bof3/world/area00813_internal.h"

extern void func_8015D404(u32 arg0, s32 arg1);
extern void func_80196070(void);

/* @source 0x801F3BB4
 * @behavior marks the shared world byte 0x80146867 with bit 0x80, additionally
 *           marking the adjacent shared byte 0x80146866 when that byte is not
 *           below 0x10, stops the shared selection effect pair (0, 2) on either
 *           path, and finally calls the shared 0x80196070 work-area reset
 *           helper.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3BB4(void) {
  u8* flags;
  u8 value;

  flags = &D_80146867;
  value = *flags;
  if (value >= 0x10u) {
    D_80146866 |= 0x80;
    *flags = value | 0x80;
    func_8015D404(0, 2);
  } else {
    *flags = value | 0x80;
    func_8015D404(0, 2);
  }
  func_80196070();
}
