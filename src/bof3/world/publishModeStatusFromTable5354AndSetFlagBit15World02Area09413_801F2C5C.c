#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u32 D_8014686C; /* @source 0x8014686C @kind unknown */
extern u16 D_801F5354[]; /* @source 0x801F5354 @kind table */

/* Shared scenario-flag bit-set helper at 0x8015B580: sets bit `bit_index` in
 * the flag word passed as the first argument. It lives outside every overlay
 * payload, so it keeps its raw generated name and takes no map row; the
 * argument spelling follows the exact siblings that call it, for example
 * setScenarioFlagBit0CScenarioScena0600_801FCF98.
 */
void func_8015B580(void* flag_bank, u8 bit_index);

/* @source 0x801F2C5C
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F5354 and
 *           publishes it into the shared status halfword D_801490A8, then sets
 *           flag bit 0x15 of the shared scenario flag word D_8014686C through
 *           the shared bit-set helper func_8015B580 when that same mode reads
 *           1. A mode of 0, or any mode other than 1, publishes the status
 *           halfword and leaves the flag word alone. Takes no arguments and
 *           returns nothing.
 *
 *           Constant and store evidence from these original bytes: the mode
 *           cell 0x801490C7 is read once with `lui`/`lb` (signed byte), the
 *           table base 0x801F5354 is indexed by `sll` by one and `addu` into
 *           $at with `lhu`, the status cell 0x801490A8 takes the `sh`, and the
 *           flag path reads the word at 0x8014686C with `lui`/`lw` into $a0
 *           before `jal func_8015B580` with 0x15 in $a1.
 *
 *           Shape: the 0x58-byte range has no byte-identical sibling anywhere
 *           in out/binaries (the span was compared word for word against all
 *           11 payloads carrying the same `jal`/`addiu $a1,0x15` pair, and
 *           every one differs), so this is a fresh clean-C seed. The statement
 *           order, the single `mode` temporary and the two-case `switch` follow
 *           the exact siblings func_801F300C, func_801F42D4 and
 *           publishModeStatusFromTable4494AndProgress1Or0AWorld02Area08013_801F3BF4,
 *           which publish the same status halfword from their own mode table
 *           through the same `beqz`/`beq` two-case dispatch. The original's
 *           `beqz $v1` first compare is what an empty case 0 leaves behind, so
 *           case 0 is written as a bare break.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusFromTable5354AndSetFlagBit15World02Area09413_801F2C5C(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F5354[mode];
  switch (mode) {
  case 0:
    break;
  case 1:
    func_8015B580(D_8014686C, 0x15);
    break;
  }
}
