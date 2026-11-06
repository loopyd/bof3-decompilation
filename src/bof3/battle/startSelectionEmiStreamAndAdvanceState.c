#include "bof3/battle/battle03_internal.h"

/* @source 0x801D7234
 * @behavior Once the EXE-side EMI loader reports ready, runs the front-end hook
 *           published at D_801463A8 and, while the D_80146325 bit-0x40 gate is
 *           clear, resets the current front-end selection effect through
 *           func_8015D5D8. A selection index other than 0xFF then stages the
 *           0x80181EB8 entry it selects into the EMI loader state record: the
 *           entry effect bytes go to D_80146855/D_80146856, the 0x20 transfer
 *           bit of D_80146854 is raised (with the 0x40 bit as well while
 *           D_80146328 carries bit 0x10), and the entry stream slot is
 *           published at D_801463B0 and started through func_80161FDC. A 0xFF
 *           selection raises bit 0x08 of D_80146328 instead. Every path that
 *           passes the loader gate advances the battle front-end state byte
 *           BATTLE_GLOBAL_BYTE_62E2 that dispatchStateTableAd20 uses to select
 *           entry 1 of the D_801EAD20 handler table, which is this handler.
 * @status partial
 * @match 95.77
 * @residual The whole 71-instruction body is present with one displaced
 *           three-instruction group: gcc 2.7.2 schedules the
 *           `lui $at,%hi(D_80181EBB)` / `addu $at,$at,$a1` /
 *           `lbu $a0,%lo(D_80181EBB)($at)` address-and-load group of the second
 *           staged byte ahead of the D_80146855 store, so it sits at offsets
 *           0x74-0x7C instead of the original 0x88-0x94 and the instruction
 *           diff starts at index 31. The original keeps that load after the
 *           D_80146855 store and the flag `ori`; sched1's ready-list order
 *           (stores carry priority 2 because their value comes from a
 *           load-delay `lbu`, loads carry 1) pulls every table load to the top
 *           of the block. Probes of the staged-byte/flag statement order and of
 *           the value representation measured 77.46-95.77% with this ordering
 *           the latest frontier: both staged bytes together with the flag store
 *           last 77.46%, the gate byte read first 92.96%, an early staged temp
 *           95.77% with first=29, one u16 temp shared by the second staged byte
 *           and the stream slot 95.77% with first=27, a named selection*4 offset
 *           neutral at first=31, and one shared 0x80181EBA base with +1 for the
 *           second byte 90.14% with a 32-byte frame. The register web, store
 *           order, constants, calls and 24-byte frame all match this candidate;
 *           no behaviour or ABI difference is known.
 */
void startSelectionEmiStreamAndAdvanceState(void) {
  volatile u8* state;
  u8 loaderState;
  u8 selection;
  u16 slot;
  u8 gate;

  if (func_80162D00() != 0) {
    D_801463A8();
    if ((D_80146325 & 0x40u) == 0u) {
      func_8015D5D8();
      selection = D_80145029;
      if (selection != 0xFFu) {
        loaderState = D_80146854;
        D_80146855 = D_80181EBA[selection * 4];
        D_80146854 = loaderState | 0x20u;
        gate = D_80146328;
        D_80146856 = D_80181EBB[selection * 4];
        if ((gate & 0x10u) != 0u) {
          D_80146854 = loaderState | 0x60u;
        }
        slot = D_80181EB8[selection * 2];
        D_801463B0 = slot;
        func_80161FDC(slot);
      } else {
        D_80146328 |= 8u;
      }
    }
    state = &BATTLE_GLOBAL_BYTE_62E2;
    *state += 1u;
  }
}
