#include "bof3/battle/battle03_internal.h"

/* @source 0x801EA650
 * @behavior Dispatches the panel task handler selected by state byte +0x03
 * through the three-entry handler table copied off 0x801D0FF8, then forwards the
 * active panel halfwords +0x4/+0x6 to func_801D7EB0 and forwards those same
 * halfwords offset by +4/+3 as signed halfwords, the constant 0xFF, and the +0xA
 * byte and +0x4 word of the 0x0C-stride event slot selected by panel byte +0xA
 * to func_8014F800.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801EA650(void) {
  Battle03DispatchTable handlers;

  handlers = D_801D0FF8;
  handlers.handlers[D_80148648[3]]();

  func_801D7EB0(FIELD_REF(s16, D_80148648, 4), FIELD_REF(s16, D_80148648, 6));
  func_8014F800((s16)(FIELD_REF(u16, D_80148648, 4) + 4u),
                (s16)(FIELD_REF(u16, D_80148648, 6) + 3u),
                D_801EB4F4[D_80148648[0xA]].unk_06, 0xffu,
                *(u32*)(void*)&D_801EB4F4[D_80148648[0xA]]);
}
