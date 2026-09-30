#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 scratch work-record handler (entry 0 of the 0x801E22F0
 * handler table, dispatched through scratch byte 4): when the shared state byte
 * D_80144281 holds 3 or 4 it masks the shared panel flag byte D_80144287 down
 * to seven bits, publishes the masked value back, submits func_801647C4 with
 * (0x1C, masked value * 16 + 0x58, 0) and advances byte 4 of the work record
 * published at the scratchpad cursor 0x1F800044. It then follows bit 0 of the
 * flag byte: the flagged low halfword 0x800100A4 or the clear-path halfword
 * 0x800100A2 of the main executable, offset by the 0x80010000 VRAM base, is
 * submitted to func_8014F800 as (0x1D, 0x14, 0, 0xFF, base + halfword).
 * @source 0x801DCCF4
 * @status exact
 * @match 100.00
 * @residual none
 * First live seed scored 46/49 (93.88%) with only the bit-0 arm polarity
 * reversed: the flag-set path must be the out-of-line arm so the clear path
 * falls through to the shared tail, matching the AREA030 sibling rule recorded
 * in the lift-loop observations. Inverting the source condition to
 * `(D_80144287 & 1u) == 0u` matched live at 49/49 instructions.
 */
void func_801DCCF4(void) {
  u8 flags;

  if (D_80144281 - 3u < 2u) {
    flags = D_80144287 & 0x7Fu;
    D_80144287 = flags;
    func_801647C4(0x1Cu, (u16)(((u32)flags << 4) + 0x58u), 0);
    D_1F800044[4]++;
  }
  if ((D_80144287 & 1u) == 0u) {
    func_8014F800(0x1Du, 0x14, 0, 0xFFu, 0x80010000u + (u32)D_800100A2);
  } else {
    func_8014F800(0x1Du, 0x14, 0, 0xFFu, 0x80010000u + (u32)D_800100A4);
  }
}
