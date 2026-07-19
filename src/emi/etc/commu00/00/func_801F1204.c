#include "internal.h"

/* D_80146904: u16 array, indexed by arg * 76 (byte offset / sizeof(u16)) */
extern u16 D_80146904[1];
extern volatile u8 D_801F2928[];

/* @source 0x801F1204
 * @behavior sets scratch slot 6, stores variant-derived u16 to task slot entry
 */
void func_801F1204(u8 task_index, u8 record_kind_index) {
    volatile s8* ptr;
    u32 offset;

    ptr = (volatile s8*)(*((void**)0x1F800044));
    ptr[6] = 1;

    offset = (task_index & 0xFF) * 76;
    D_80146904[offset] = (u16)(D_801F2928[record_kind_index & 0xFF] - 0x3FFB);
}
