#include "bof3/bof3.h"

typedef void (*ScenarioScena0800WorkStateHandler)(void);

extern ScenarioScena0800WorkStateHandler D_801FE8E4[];
void func_801F8D98(u8* buffer);
void func_801F90C8(u8* buffer);

/* @source 0x801F7AD0
 * @behavior Reads the scratchpad work pointer at 0x1F800044 and dispatches
 * its unsigned byte at offset 2 through the overlay-local handler table at
 * 0x801FE8E4, passing no arguments. Then calls 0x801F8D98 and 0x801F90C8,
 * in that order, each with the buffer at 0x800E48E0. The first helper shifts
 * 24-byte records and computes their projected coordinates and angles; the
 * second consumes those records to emit graphics primitives. Helper return
 * values are discarded. This wrapper takes no arguments and returns nothing;
 * its 24-byte stack frame preserves only the return address. The handler table
 * begins with 0x801F7B2C, 0x801F7BA8, 0x801F7C08, 0x801F7C98 and 0x801F7CF8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenProcessBufferScenarioScena0800_801F7AD0(void) {
  D_801FE8E4[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
  func_801F8D98(PSX_PTR(u8, 0x800E48E0u));
  func_801F90C8(PSX_PTR(u8, 0x800E48E0u));
}
