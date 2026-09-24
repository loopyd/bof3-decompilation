#include "bof3/bof3.h"

extern s8 D_80146872;
extern u32 D_801F6D6C[];

/* @source 0x801F6C04
 * @behavior Dispatches this overlay's per-frame handler named by the signed progress
 * byte D_80146872 through the overlay's own four-byte handler table at 0x801F6D6C.
 */
void dispatchProgressHandler_scena18(void) {
  ((void (*)(void))D_801F6D6C[(s8)D_80146872])();
}
