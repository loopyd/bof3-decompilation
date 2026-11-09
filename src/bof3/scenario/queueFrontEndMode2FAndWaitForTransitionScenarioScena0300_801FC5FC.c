#include "bof3/bof3.h"

/* Shared main-RAM cells. Both sit below this overlay's 0x801F6C00 load address
 * and resolve through this target's generated undefined-symbol map. */
extern u8 D_80145029; /* @source 0x80145029 @kind data */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Shared main-EXE helpers, all outside this payload and unlifted, so only the
 * call-site prototypes are declared. */
void func_80166180(s32 arg0);
void func_80150224(s32 arg0);
void func_80161D20(s32 arg0);
void func_801636A0(u32 arg0, u32 arg1);
u32 func_80163EA0(void);
void func_801992B8(void);
void func_80161D74(s32 arg0);

/* @source 0x801FC5FC
 * @behavior Handler slot 5 of the overlay record-callback run at 0x801FD00C,
 *           which dispatchRecordCallbackScenarioScena0300_801FC46C selects with
 *           the record byte at offset 0x7A and calls as (record, D_8014686C);
 *           the record pointer and the shared word are never read and the
 *           callback return value is discarded, while the 0x18-byte frame only
 *           keeps $ra across the calls. It first marks shared selector id 1
 *           through the main-EXE setter 0x80166180, the write half of the
 *           query pair whose read half 0x80166140 the sibling overlays test
 *           with the same id. It then queues front-end mode 0x2F through the
 *           shared dispatcher 0x80150224 and latches the shared front-end
 *           phase byte D_80143BB0 to 2. Next it hands the shared selection
 *           index byte D_80145029, loaded unsigned, to the shared helper
 *           0x80161D20 and queues the shared two-argument effect group 2 with
 *           the flag 1 through 0x801636A0. The rest of the handler polls the
 *           main-EXE frontend-transition predicate 0x80163EA0, whose body
 *           reports nonzero while the signed counter at 0x8014682D reads zero
 *           or below: while its masked low byte is zero the handler runs the
 *           shared helper 0x801992B8 and yields one callback-scheduler count
 *           through 0x8014B87C with 1, and the first nonzero report leaves the
 *           loop, at which point it loads D_80145029 a second time and hands it
 *           to the shared helper 0x80161D74 before returning.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueFrontEndMode2FAndWaitForTransitionScenarioScena0300_801FC5FC(void) {
  func_80166180(1);
  func_80150224(0x2F);
  D_80143BB0 = 2;
  func_80161D20(D_80145029);
  func_801636A0(2, 1);
  while ((u8)func_80163EA0() == 0) {
    func_801992B8();
    func_8014B87C(1);
  }
  func_80161D74(D_80145029);
}
