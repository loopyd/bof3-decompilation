#include "internal.h"

/* @behavior advances the two frontend window fades, promotes the fade phase
 * once both channels saturate, and draws the visible menu/window layers.
 * @source 0x801D1B00
 */
void func_801D1B00(void) {
  /* MATCHING_AID: the draw calls pass the low byte of each u16 alpha global
   * with a single symbol-relative `lui/lbu` (e.g. `lui a1 ; lbu a1,%lo(a1)`),
   * reloading it per call. Reading `(u8)GLOBAL` of the volatile u16 emits
   * `lhu+andi`, and `*(u8*)&GLOBAL` / PSX_REF materialize a persistent pointer
   * (lui+addiu/ori into a saved register) and load off it. The raw u8 aliases
   * D_80143C26/D_80143C28 (same address as the u16 globals, bound in symbols.c
   * and resolved by their hex suffix) make cc1 treat the access as a u8
   * variable read, emitting the ephemeral symbol-relative lbu the original
   * uses. They are not map entries: the address-keyed map allows one name per
   * address, so a same-address alias resolves by name suffix instead. */
  u8* phase;
  u8  p;
  /* MATCHING_AID: the two fade-channel active flags are u8 locals held in the
   * original's saved registers (secondary -> s0, primary -> s1). With u8 flags
   * cc1 loads a fresh `li v0,1` for the `p == 1` dispatch compare (filling the
   * lbu load-delay slot) instead of reusing s1, and saves s0 right after
   * `li s1,1` in the prologue — both matching the original. An s32 flag makes
   * cc1 fold the constant 1 out of s1 (`nop ; bne v1,s1`) and reorder the save.
   * The promote test and draw calls still emit `andi ...,0xff` because the u8
   * value lives in a 32-bit register used in a word context. */
  u8 primary_active;
  u8 secondary_active;
  /* MATCHING_AID: the fade increment is a full-word local. The original keeps
   * the incremented value in v0 across the store and sign-extends v0 in place:
   * `lhu v0 ; addiu v0,4 ; sh v0 ; sll v0,16 ; sra v0,16 ; slti v0,128`. A u16
   * local makes cc1 split the store and compare webs (`move v1,v0 ; sh v1`);
   * an s32 local keeps one 32-bit register so `sh` stores the low half and the
   * `(s16)` cast sign-extends the same register, move-free. */
  s32 alpha;

  primary_active = 1;
  secondary_active = 1;
  phase = (u8*)&GAME_FRONT_WINDOW_PHASE;

  p = *phase;
  if (p == 1u) {
    alpha = GAME_FRONT_WINDOW_ALPHA_PRIMARY + 4u;
    GAME_FRONT_WINDOW_ALPHA_PRIMARY = (u16)alpha;
    barrier();
    if ((s16)alpha >= 128) {
      GAME_FRONT_WINDOW_ALPHA_PRIMARY = 128u;
      primary_active = 0;
    }

    alpha = GAME_FRONT_WINDOW_ALPHA_SECONDARY + 2u;
    GAME_FRONT_WINDOW_ALPHA_SECONDARY = (u16)alpha;
    barrier();
    if ((s16)alpha >= 128) {
      GAME_FRONT_WINDOW_ALPHA_SECONDARY = 128u;
      secondary_active = 0;
    }

    if (primary_active == 0u && secondary_active == 0u) {
      *phase = 2u;
    }
  } else if (p == 2u) {
    primary_active = 0;
    secondary_active = 0;
    GAME_FRONT_WINDOW_ALPHA_PRIMARY = 128u;
    GAME_FRONT_WINDOW_ALPHA_SECONDARY = 128u;
  }

  if (GAME_FRONT_WINDOW_PHASE != 0u) {
    func_801D12CC(secondary_active, D_80143C28);
    func_801D16DC(26, 24, secondary_active, D_80143C28);
    /* MATCHING_AID: the original passes primary_active to func_801D150C's
     * selected parameter as a full word (`move a2,s1` in the jal delay slot),
     * with no andi 0xff truncation, even though the declared parameter is u8.
     * Calling through a widened s32 prototype reproduces that word-wide
     * argument pass: cc1 would otherwise narrow the u8 value with
     * `andi a2,...,0xff`. The callee (byte-matched) reads only the low byte,
     * so the wider pass is behavior-identical. */
    ((void (*)(s16, s16, s32, u8))func_801D150C)(-6, 28, primary_active,
                                                 D_80143C26);
  }
}
