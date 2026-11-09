#include "bof3/bof3.h"

/* Shared front-end state-entry request byte of the main executable, outside
 * this overlay's payload: the state dispatcher indexes its handler table with
 * it. Read here with `lb`, so the declaration keeps the s8 width that
 * src/bof3/world/initArea10513State.c gives it. */
extern s8 D_801448EB; /* @source 0x801448EB @kind data */

/* Staged and live copies of the front-side reading: 0x80143F06 is the copy of
 * 0x80143F03 that the front-entry staging routine at 0x8019FAA0 publishes at
 * its 0x8019FB48 store. Both are read here with `lbu`, so both stay u8; the
 * shared map row binds D_80143F03 and this target's generated undefined-syms
 * row binds D_80143F06. */
extern u8 D_80143F06;
extern u8 D_80143F03;

/* Overlay banner routine at 0x801F3994, still an `asm` span of this target. It
 * tests flag 0x7E of the shared flag bank 0x80144F28 through func_8015B5D4 and
 * queues banner glyph cells through func_801A4BC0; the `jal` that reaches it
 * here carries only a dead leftover address in $a0, so the prototype takes no
 * arguments. */
void func_801F3994(void);

/* Shared frontend cue dispatcher of the main executable, outside this
 * overlay's payload; the cue id arrives zero-extended, as the sibling
 * declarations record. */
void func_8015DF18(u16 arg0);

/* @source 0x801F4ABC
 * @behavior Overlay step handler in two parts. It first remaps the shared
 *           front-end state-entry request byte D_801448EB: while that byte
 *           holds 0x33 it is replaced by 0x25, the same state entry the
 *           sibling setters of this overlay request, and any other value is
 *           left alone. It then runs the overlay banner routine at 0x801F3994
 *           unconditionally. The cue is the last part: when the staged
 *           front-side byte D_80143F06 reads 2 with the live byte D_80143F03
 *           reading 1, or when the staged byte reads 3 with the live byte
 *           reading 2, the handler requests frontend cue 0x20C through
 *           func_8015DF18. The staged byte is reloaded for the second pair,
 *           because the first pair can reach the cue call and the byte is
 *           re-read after it. Takes no arguments and returns nothing; the
 *           0x18-byte frame only keeps $ra at 0x10.
 *
 * Caller: the payload holds no `jal` to this address, only one descriptor
 * word. The overlay's trailing data put 0x801F4ABC at payload 0x38B4 as the
 * second word of the descriptor pair at 0x38B0, whose first word 0x801F646C
 * points into the six-entry setter table at 0x801F645C, so the invocation is
 * the overlay script's and the argument roles stay structural.
 *
 * Name evidence: the two stores are the state-entry remap 0x33 to 0x25 and the
 * gated front cue 0x20C; the state byte keeps the role that
 * setStatusFFFFAndStateEntry25Progress5WhenGateClearWorld04Area17013_801F4950
 * records for 0x801448EB, and the two byte pairs are the crossed front-side
 * reading that selectCrossedFrontSideStateWorld04Area18813_801F4D88 reads from
 * the same two cells. The banner routine's own role is beyond this span, so
 * the name describes what this handler does instead of naming that callee.
 *
 * Shape: the crossed front-side arms are the two-arm statement order of
 * selectCrossedFrontSideStateWorld04Area18813_801F4D88, and the unguarded call
 * between a state byte store and a draw callee is the order of
 * advanceWorkByte2AndPlayCue200WhenFlag67SetThenDrawWorld04Area17013_801F4574.
 * Neither sibling is byte-identical: a payload comparison of this 0x9C-byte
 * range against all 396 shipped payloads, masking the three `jal` immediates,
 * finds only this span, so the clone lever is ineligible and the body is
 * written from this target's own bytes. The first seed addressed D_801448EB
 * directly and measured 34/39 with `lui v1` plus a `lui at` store base at
 * +0x0004; a sweep of the state-access shapes selected the pointer local,
 * which reproduces the single base register of the original at 39/39.
 * @status exact
 * @match 100.00
 * @residual none
 */
void replaceStateEntry33With25AndDrawBannerAndQueueCue20COnCrossedFrontSideWorld04Area17013_801F4ABC(
    void) {
  s8* state;

  /* The original materialises the whole address of D_801448EB into one
   * register and reaches both the `lb` and the `sb` through that base, so the
   * state byte is taken through a pointer local instead of two separate
   * `lui`/`%lo` pairs. */
  state = &D_801448EB;
  if (*state == 0x33) {
    *state = 0x25;
  }
  func_801F3994();
  if (D_80143F06 == 2) {
    if (D_80143F03 == 1) {
      func_8015DF18(0x20C);
    }
  }
  if (D_80143F06 == 3) {
    if (D_80143F03 == 2) {
      func_8015DF18(0x20C);
    }
  }
}
