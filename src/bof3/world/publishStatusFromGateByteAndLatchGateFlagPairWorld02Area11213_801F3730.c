#include "bof3/bof3.h"

/* Shared gate byte this step publishes and switches on. The status expression
 * below narrows it through a signed char, while the switch reads it signed. */
extern s8 D_801490C7; /* @source 0x801490C7 @kind data */

/* Shared status halfword this step publishes. */
extern u16 D_801490A8; /* @source 0x801490A8 @kind data */

/* Shared main-RAM flag bank whose bits 0x33, 0x34 and 0x35 this step drives. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Shared flag record whose bit 0x10 gates the release step and whose flag bank
 * sits 0x38 bytes into it. */
extern u8 D_80144EF0; /* @source 0x80144EF0 @kind data */

extern void func_8015B580(void *flag_bank, u8 bit_index); /* @source 0x8015B580 */
extern void func_8015B5A8(void *flag_bank, u8 bit_index); /* @source 0x8015B5A8 */
extern s32 func_8015B5D4(u32 flag_base, s32 bit_index);   /* @source 0x8015B5D4 */

/* @source 0x801F3730
 * @behavior Publishes the shared status halfword 0x801490A8 from the shared
 *           gate byte 0x801490C7 plus three, so gate values zero through three
 *           publish statuses 3 to 6 and gate 4 publishes 0xFFFF. The step also
 *           raises bit 0x33 of the shared flag bank 0x80144F28 and mirrors the
 *           two low bits of the gate byte into bits 0x34 and 0x35 of that bank:
 *           gate 0 clears both, gate 1 clears 0x34 and raises 0x35, gate 2
 *           raises 0x34 and clears 0x35, and gate 3 raises both. The five-word
 *           jump table the payload keeps at 0x801F2C04 selects those arms, and
 *           the arm an out-of-range gate reaches is the all-clear arm. When bit
 *           0x10 of the shared flag record 0x80144EF0 reads clear through
 *           func_8015B5D4, the step republishes status 2 and clears bit 0x33
 *           through the bank reached from that record base plus 0x38, which is
 *           the same 0x80144F28 bank; the original therefore keeps the record
 *           base live in $s0 and adds 0x38 instead of materialising the bank
 *           symbol a second time.
 *
 * Evidence: the payload span 0x801F3730-0x801F386F is one complete function of
 * 0x140 bytes with a single 0x18-byte frame and one `jr $ra` epilogue, so it is
 * neither data nor a fragment, and the handler pointer run at 0x801F48AC
 * records its address. The original sign-extends the first gate read with the
 * `lbu` plus `sll 24`/`sra 24` pair, reads it again with a plain `lb` for the
 * dispatch, bounds that value with `sltiu 0x5` without a minimum subtraction
 * and indexes the five-word table by four times the value, which fixes the
 * label set at 0 to 4 with the all-clear arm carrying the default. The four
 * arms share their second call through two cross-jumped tails, and the record
 * test reuses the value of `func_8015B5D4` in the delay slot of the branch that
 * skips the release.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishStatusFromGateByteAndLatchGateFlagPairWorld02Area11213_801F3730(
    void) {
  u8 *record;

  D_801490A8 = D_801490C7 + 3;
  func_8015B580(&D_80144F28, 0x33);
  switch (D_801490C7) {
  case 0:
  default:
    func_8015B5A8(&D_80144F28, 0x34);
    func_8015B5A8(&D_80144F28, 0x35);
    break;
  case 1:
    func_8015B5A8(&D_80144F28, 0x34);
    func_8015B580(&D_80144F28, 0x35);
    break;
  case 2:
    func_8015B580(&D_80144F28, 0x34);
    func_8015B5A8(&D_80144F28, 0x35);
    break;
  case 3:
    func_8015B580(&D_80144F28, 0x34);
    func_8015B580(&D_80144F28, 0x35);
    break;
  case 4:
    D_801490A8 = 0xFFFF;
    break;
  }
  record = &D_80144EF0;
  if (func_8015B5D4((u32)record, 0x10) == 0) {
    D_801490A8 = 2;
    func_8015B5A8(record + 0x38, 0x33);
  }
}
