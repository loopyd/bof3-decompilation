#include "bof3/bof3.h"

/* Row table of this overlay's payload, seven rows of four bytes at
 * 0x801F5CC0: the two key bytes 0x3E/0x39 and 0x5D,0x64,0x63,0x5F,0x5E,0x69,
 * 0x68, the category nibble 1,7,7,7,7,7,7 (matched against the leading byte of
 * the main-RAM game-mode record) and the sub-state 0x0A,0x02,0x02,0x04,0x04,
 * 0x06,0x06. The original reaches each column through its own absolute symbol
 * with a shared byte offset register, so the columns keep separate array
 * declarations instead of a row view.
 * @source 0x801F5CC0 @kind table */
extern u8 D_801F5CC0[];
/* @source 0x801F5CC1 @kind table */
extern u8 D_801F5CC1[];
/* @source 0x801F5CC2 @kind table */
extern u8 D_801F5CC2[];
/* @source 0x801F5CC3 @kind table */
extern u8 D_801F5CC3[];

/* First main-RAM flag bank tested through func_8015B5D4. The original forms the
 * bank address with one `lui`/`addiu` pair and passes it whole, so the cell
 * keeps byte type and its address is what the helper receives.
 * @source 0x80144EF0 @kind unknown */
extern u8 D_80144EF0;

/* Leading byte of the main-RAM game-mode record, read with one `lbu` at the
 * record base as the category the matched row's third column must agree with.
 * @source 0x80145E98 @kind unknown */
extern u8 D_80145E98;

/* Shared state-entry selector: dispatchIndexedStateHandler indexes the
 * D_801C84BC handler table with this byte, and the world overlays publish their
 * next state entry here.
 * @source 0x801448EB @kind unknown */
extern u8 D_801448EB;

/* Shared sub-state byte published together with the state entry: the same pair
 * the scenario overlays write as primary state and sub-state.
 * @source 0x801448EC @kind unknown */
extern u8 D_801448EC;

/* Shared two-stage flag wait of the main executable: it reports whether the
 * flag of the bank it is given is set, taking the bank base and the flag id.
 * Prototype and argument types follow the exact sibling that reaches it from
 * the same family. */
s32 func_8015B5D4(u32 arg0, s32 arg1);

/* Shared front-end startup helper; takes no arguments and answers nothing. */
void func_8015C088(void);

/* @source 0x801F4504
 * @behavior Overlay entry gate of the primary state 0x14 family. Both byte
 *   arguments are zero-extended once, outside the walk, and matched against the
 *   seven four-byte rows of the payload table at 0x801F5CC0 (key bytes
 *   0x3E/0x39 and 0x5D,0x64,0x63,0x5F,0x5E,0x69,0x68, category nibble
 *   1,7,7,7,7,7,7, sub-state 0x0A,0x02,0x02,0x04,0x04,0x06,0x06). A row is
 *   taken when its first byte equals the first argument, its second byte the
 *   second argument and the low nibble of its third byte the leading byte of
 *   the main-RAM game-mode record 0x80145E98, the same low-nibble category gate
 *   the record walkers of this project use. When the walk runs past the seventh
 *   row, or when the shared two-stage flag wait func_8015B5D4(&D_80144EF0, 9)
 *   reports the flag clear, it answers 0; otherwise it runs the shared front-end
 *   startup helper func_8015C088, publishes primary state 0x14 in D_801448EB
 *   with the matched row's fourth byte as the sub-state in D_801448EC, and
 *   answers 1. The 0x18-byte frame keeps $s0 - both the row counter and the row
 *   index the sub-state column is read with - and $ra. No word and no jal of any
 *   overlay holds 0x801F4504, so this handler is reached from outside the
 *   payload.
 *
 * Name evidence: 0x801448EB is the state-entry selector that etc/game/00
 *   dispatches its D_801C84BC handler table through (dispatchIndexedStateHandler
 *   reads it as a table index) and that the world overlays publish their next
 *   state entry into; the scenario overlays write the same pair as primary state
 *   and sub-state, so 0x14 names the family and D_801448EC the row sub-state.
 *   0x80144EF0 with the id 9 is the bank/id pair of the shared flag wait
 *   func_8015B5D4, the helper the same families call to test one flag bit (the
 *   sibling at 0x801E4C2C reads flag 0x6C of the neighbouring bank 0x80144F28
 *   the same way). The arguments carry no evidenced role beyond table keys, so
 *   the name keeps them structural.
 *
 * Shape: the walk keeps one named byte-offset local `offset = i * 4` shared by
 *   all four columns, so each column load stays a `symbol + register` address
 *   and re-forms the original `lui $at,%hi` / `addu $at,$at,$a2` /
 *   `lbu %lo($at)` triple per access; a row-typed or per-column pointer spelling
 *   hoists one base register per column instead and measured 35 of 58
 *   instructions at 244 bytes. The game-mode byte is read through a volatile
 *   pointer so the original's per-iteration `lbu` stays in the walk while its
 *   address is hoisted into $a3; a plain scalar read hoisted the loaded value
 *   instead. The two zero exits are one combined guard, and the flag bank is
 *   passed as its address, so both exits branch to the shared epilogue with the
 *   zero in the branch delay slot and the success path falls through into it
 *   with `addiu $v0,$zero,1` as the original does; two separate `return 0`
 *   statements measured 53 of 58 instructions at 240 bytes with a duplicated
 *   zero block and a `j`/`li` success exit, and a word-valued bank cell added
 *   the `lw` form instead of the address form.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestPrimaryState14WithMatchedRowSubstateWhenFlag9SetWorld03Area14514_801F4504(
    u8 arg0, u8 arg1) {
  s32 i;
  s32 offset;

  for (i = 0; i < 7; i++) {
    offset = i * 4;
    if (arg0 != D_801F5CC0[offset]) {
      continue;
    }
    if (arg1 != D_801F5CC1[offset]) {
      continue;
    }
    if ((D_801F5CC2[offset] & 0xF) != *(volatile u8*)&D_80145E98) {
      continue;
    }
    break;
  }

  if (i == 7 || func_8015B5D4((u32)&D_80144EF0, 9) == 0) {
    return 0;
  }

  func_8015C088();
  D_801448EB = 0x14;
  D_801448EC = D_801F5CC3[i * 4];
  return 1;
}
