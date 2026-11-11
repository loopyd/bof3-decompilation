#include "bof3/bof3.h"

extern u32 D_8014686C;
extern u8 D_801448EB;
extern u8 D_801448EC;
extern u8 D_801448ED;

void func_8015B580(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FB414
 * @behavior Sets bit 0x1C of the shared scenario flag word held at 0x8014686C
 * through the shared bit-set helper func_8015B580, runs the shared front-end
 * startup helper func_8015C088, and then requests shared state entry 0x2C by
 * writing the state-entry selector byte at 0x801448EB, progress 0 into the
 * byte at 0x801448EC and 0x11 into the companion byte at 0x801448ED. The flag
 * word is read with `lui`/`lw` into $a0 and the bit index materialises in $a1
 * with `addiu`; both call delay slots stay nops because the bit helper reads
 * its arguments from registers and the startup helper takes none. The
 * 0x18-byte frame only keeps $ra across the two calls and its save is
 * scheduled into the gap between the flag-word load and the first call, each
 * byte store materialises the 0x8014 page in $at on its own, the entry
 * constant 0x2C needs its own `addiu`, the progress store writes $zero and the
 * companion constant 0x11 is materialised above that store, which is how the
 * psyq compiler emits a three-cell tuple of two literals around a zero. Takes
 * no arguments and returns nothing, and it reads none of them.
 *
 * Name evidence: 0x801448EB is the state-entry selector of the shared tuple,
 * the byte the shared dispatcher indexes its handler table with
 * (dispatchIndexedStateHandler reads it as `D_801C84BC[(s32)D_801448EB]()`),
 * with 0x801448EC as the progress byte and 0x801448ED as its companion
 * sub-state byte in the setStatusFFFFAndStateEntry* family vocabulary;
 * 0x8014686C is the shared scenario flag word the family's flag helpers pass
 * to 0x8015B580, 0x8015C088 is the shared front-end startup helper the whole
 * world family runs, and 0x8015B580 is the shared bit-set helper of the main
 * executable. The payload names this body only from a table word: no in-image
 * `jal` reaches 0x801FB414, its only image word is 0x801FBFC4, entry 3 of the
 * thirteen-entry null-terminated code-pointer run at 0x801FBFB8 (0x801FBFB8
 * through 0x801FBFE8) that dispatchRecordCallbackScenarioScena1300_801FB350
 * (0x801FB350) indexes by record byte 0x7A, so this span is a record callback
 * and its argument roles stay structural.
 *
 * Shape: statement order, operand forms and declaration style are taken from
 * two exact siblings. The three-cell tuple is byte-shape identical, apart from
 * its own literals, to the tail of
 * setStatusFFFFAndStateEntry2BSubstate1WhenGateClearWorld04Area18813_801F4670
 * (0x801F4670, world04/area188/13), which emits the same
 * `addiu`/`lui`/`sb` sequence with its next constant hoisted above the zero
 * progress store; the leading
 * func_8015B580(flag word, bit)/func_8015C088 pair and its two-call frame are
 * the pair
 * setStatusFFFFAndStateEntry3BProgress0SetSharedFlagBit8FWhenGateClearWorld04Area16813_801F4920
 * (0x801F4920, world04/area168/13) runs before its own tuple, and the
 * `extern u32` cell plus `func_8015B580(u32, s32)` declaration style comes from
 * the exact lift setScenarioFlagBit3CAndQueueCue203World03Area11913_801F4524
 * (0x801F4524, world03/area119/13). Every literal here is written from this
 * span's own bytes.
 *
 * Clone declined: an exact scan of the 0x50-byte body over every extracted
 * payload under out/binaries returned only this span's own offset 0x4814 in
 * its own payload, and a second scan with the `jal` target word masked so
 * relocation-different siblings could match returned nothing more, so no twin
 * exists and the siblings above are shape evidence rather than a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScenarioFlagBit1CThenRequestStateEntry2CProgress0Substate11ScenarioScena1300_801FB414(
    void) {
  func_8015B580(D_8014686C, 0x1C);
  func_8015C088();
  D_801448EB = 0x2C;
  D_801448EC = 0;
  D_801448ED = 0x11;
}
