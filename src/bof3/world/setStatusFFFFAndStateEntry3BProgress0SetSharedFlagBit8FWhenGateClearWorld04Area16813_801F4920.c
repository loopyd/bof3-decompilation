#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_80144F28[];
extern u8 D_801448EB;
extern u8 D_801448EC;

/* Shared bit helper of the main executable: writes the bit its second argument
 * selects in the flag bank its first argument addresses. This target's own
 * exact lifts of the neighbouring handlers reach the same helper.
 */
void func_8015B580(void* bitmap, s32 bit_index);

/* Shared front-end startup helper of the main executable, also reached from the
 * neighbouring handlers of this target.
 */
void func_8015C088(void);

/* @source 0x801F4920
 * @behavior Stores 0xFFFF into the shared status halfword at 0x801490A8 on
 *           every path and, while the shared gate byte at 0x801490C7 is zero,
 *           sets bit 0x8F of the shared flag bank at 0x80144F28 through the
 *           shared bit helper func_8015B580, runs the shared front-end startup
 *           helper func_8015C088, then requests state entry 0x3B through the
 *           shared state byte at 0x801448EB with progress 0 in the shared byte
 *           at 0x801448EC. The gate is read with `lb` (so the cell is s8) and
 *           that load is hoisted above the status store; `bnez` jumps to the
 *           epilogue, so the gate-clear path falls through its two byte stores
 *           into the frame restore. The 0x18-byte frame only keeps $ra across
 *           the two helper calls, and both call delay slots stay nops because
 *           the flag helper reads its arguments from registers and the startup
 *           helper takes none. The flag bank address materialises in $a0 with
 *           `lui`/`addiu` and the bit index in $a1 with `addiu`, each byte store
 *           of the entry tuple materialises the 0x8014 page in $at on its own,
 *           the 0x3B entry constant needs its own `addiu`, and the progress
 *           store writes $zero. Takes no arguments and returns nothing.
 *
 * Name evidence: 0x801448EB is the state-entry byte of the shared tuple the
 * setStatusFFFFAndStateEntry* handlers write and 0x801448EC its progress byte;
 * the in-target exact lift
 * setStatusFFFFAndStateEntry2BSubstate1WhenGateClearWorld04Area18813_801F4670
 * shows the same zero progress store as `$zero`, and the near-identical
 * World04 area170 sibling below carries 0x25 with progress 5. 0x801490C7 is the
 * shared gate byte those handlers branch on, 0x8015C088 is the shared front-end
 * startup helper the world family runs, and 0x80144F28 is the main-RAM flag bank
 * that the shared bit helper 0x8015B580 writes for every handler of this family
 * (overlay World02 area108 target emi/world02/area108/14 reaches the same bank
 * as `D_801448ED + 0x63B`, overlay World03 area140 target emi/world03/area140/13
 * passes the bank base itself). The payload names this body only from a table
 * word: 0x801F4F44 holds 0x801F4920 inside the handler run
 * 0x801F4F30..0x801F4F4C of the script blob, and no in-image `jal` reaches it,
 * so its argument roles stay structural and none are taken.
 *
 * Shape: statement order, operand forms and declaration style are taken from the
 * in-target exact sibling
 * setStatusFFFFAndStateEntry3BProgressAWhenGateClearWorld04Area16813_801F4980,
 * whose 0x54-byte body is this 0x60-byte span minus the flag-bank call and with
 * 0x0A in place of this span's zero progress, so that body carried the order of
 * the status store, the hoisted gate load, the startup call and the entry tuple
 * while every literal here is written from this span's own bytes. The inserted
 * `func_8015B580(D_80144F28, 0x8F)` statement takes its operand form from
 * clearWorkByte74Bit0AndSetSharedFlagBankBit54ThenResetScratchWorkByteWhenWorkRecordKind5To7World03Area14013_801F3DAC,
 * an exact lift that passes the same bank base and bit index pair.
 *
 * Clone declined: a scan of all 396 extracted payloads under out/binaries for
 * this 0x60-byte body, unmasked and again with its `jal` and `lui` words masked
 * so relocation-different siblings could match, returned only this span's own
 * offset in its own payload, so no byte-identical twin exists and the sibling
 * above is shape evidence rather than a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndStateEntry3BProgress0SetSharedFlagBit8FWhenGateClearWorld04Area16813_801F4920(
    void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_8015B580(D_80144F28, 0x8F);
    func_8015C088();
    D_801448EB = 0x3B;
    D_801448EC = 0;
  }
}
