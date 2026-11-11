#include "bof3/bof3.h"

/* Work-area record pointer cell of the shared main executable, outside this
 * overlay's payload; the exact world01 area048 lift that reads the same cell
 * records it immediately before the D_80146888 work-area array. Records
 * published through it are 0x140 bytes long: this body reads the byte +0x74 and
 * the halfword +0x7E that the neighbouring world lifts read at the same
 * offsets, the animation-flag byte and the countdown halfword.
 * @source 0x80146884 @kind data
 */
extern u8* D_80146884;

/* Byte +0x79 of the first 0x140-stride local work record of the shared run at
 * 0x80145E90: the record's party byte. The scenario and bmagic lifts outside
 * this overlay read the same byte as that record's party id, and the world01
 * area048 lift tests it the same way, so it stays a plain byte here.
 * @source 0x80145F09 @kind data
 */
extern u8 D_80145F09;

/* Records 0x80144968 + n * 0xA4 of the shared 0xA4-byte-stride record table,
 * seen from the status byte +0x0E of record 5: 0x80144968 + 5 * 0xA4 + 0x0E is
 * this address. The scenario lift at 0x801FDF84 names the same table's record 4
 * a character record and writes its +0x0E..+0x11 status bytes, so this body
 * reads record 5's first status byte. Adding 0x27E to the same base yields the
 * main-RAM flag bank 0x80144F28 that the body hands to the flag helper, which
 * is how the original materialises that bank (the base register is reused and
 * the low word becomes the 0x27E delta), so the bank is written through this
 * byte-run base rather than as a second symbol.
 * @source 0x80144CAA @kind unknown
 */
extern u8 D_80144CAA[];

/* Front-end state entry and its progress byte of the shared main executable.
 * The ui lifts dispatch through the state entry at 0x801448EB and step the
 * progress byte at 0x801448EC, and the world lifts request an entry/progress
 * pair by storing the two bytes, which is what this body does.
 * @source 0x801448EB @kind data
 */
extern u8 D_801448EB;
/* @source 0x801448EC @kind data */
extern u8 D_801448EC;

/* Shared main-RAM flag helper (flag bank, numbered flag). The world01 area026,
 * world03 area119 and etc/sisyou/00 lifts pass a bank base and a flag number,
 * and this body passes the flag bank 0x80144F28 with flag 0x73.
 * @source 0x8015B580
 */
extern void func_8015B580(void* arg0, u8 bit_index);

/* Shared UI marker queue (x, y, size) that the neighbouring world04 area188
 * lifts call with literal cell coordinates and sizes.
 * @source 0x801A4BC0
 */
extern void func_801A4BC0(s16 x, s16 y, u32 size);

/* Shared front-end startup helper of the main executable, run by the
 * neighbouring scenario handlers before they arm a front-end state entry.
 * @source 0x8015C088
 */
extern void func_8015C088(void);

/* @source 0x801F3558
 * @behavior Overlay handler named only by data: the payload word at 0x801F413C
 *           holds 0x801F3558, word 1 of the code-pointer run that starts at
 *           0x801F4138 with 0x801F34F8 and continues at 0x801F4140 with
 *           0x801F3650, and no jal of this payload targets it, so the overlay
 *           loader or that run's dispatcher enters it and its roles stay
 *           structural. It clears bit 0
 *           of the byte at +0x74 of the work record published by the pointer
 *           cell 0x80146884 with an `andi 0xFE` read-modify-write. When the
 *           shared work record's party byte 0x80145F09 reads 5 it first tests
 *           the status byte +0x0E of character record 5 of the 0x80144968 table:
 *           when that byte reads 0x4B it sets flag 0x73 of the main-RAM flag
 *           bank 0x80144F28, queues the four shared UI markers of the block at
 *           y = 0x21 and 0x22 with size 0xC0 at x = 2 and size 0 at x = 3, and
 *           adds one to the work record's countdown halfword at +0x7E; a
 *           status byte other than 0x4B returns from the body at once, so that
 *           edge jumps to the shared epilogue and skips the party-7 test. When
 *           the party byte reads 7 it runs the shared front-end startup helper
 *           func_8015C088 and then requests front-end state entry 0x30 with
 *           progress 0xA. The two party tests are independent reads of the same
 *           byte, so the second one is taken after the first block as well. Takes
 *           no arguments and returns nothing; the 0x18-byte frame keeps $s0 for
 *           the party-byte address across the six calls, and the work record
 *           pointer is re-read from its cell after those calls.
 *
 * Matching shape: the party byte is read twice through one pointer local
 *   (`party = &D_80145F09`), which is what keeps that address in $s0 across the
 *   six calls; writing the two tests as direct reads of the global made gcc
 *   materialise the address at each use instead and lose the $s0 save (measured
 *   53/62 instructions, 244 bytes, first difference +0x0C). Because the
 *   status-byte edge leaves the function, the block is written as an early
 *   `return` on `!= 0x4B`; the positive-guarded form measured 61/62 instructions
 *   at 248 bytes with first difference +0x48, where that edge landed on the
 *   party-7 test instead of the epilogue.
 *
 * Naming evidence: the body carries every fact the name states, and each role
 *   comes from an exact lift elsewhere in the tree rather than from this span
 *   alone. The +0x74 bit-0 clear is the same statement, in the same order
 *   against the same hoisted pointer load, as the exact world01 area048 lift
 *   clearWorkByte74Bit0ThenIncrementModeByteBelow8AndQueueCue20DWhenMappedStateIs1-
 *   World01Area04813_801F4864, which also names 0x80145F09 the record's party/id
 *   byte. The flag helper call follows the world03 area119 and etc/sisyou/00
 *   declarations of 0x8015B580 as (bank, flag index), and its bank argument
 *   0x80144CAA + 0x27E is the main-RAM flag bank 0x80144F28 of the exact ui
 *   lifts clearFlag1CAndWorkFlagsWhenCountdownExpires and func_801E4C2C. The
 *   four marker calls reuse the (x, y, size) grammar of the exact world04 area188
 *   sibling queueSharedUiElementBlock1CTo1Dx3SizeC0And20x15To16SizeA1World04
 *   Area18813_801F4A54 with this body's own literals 2, 3, 0x21, 0x22, 0xC0 and
 *   0, and the immediately following span 0x801F3650 queues the same 2/3 by
 *   0x21/0x22 block with its own size 0x89. The front-end pair 0x801448EB and
 *   0x801448EC is the entry/progress pair of the exact world lifts
 *   setStatusFFFFAndStateEntry2BSubstate1WhenGateClearWorld04Area18813_801F4670
 *   and its neighbours. The status byte offset follows the character record of
 *   the exact scenario lift addExp10000ToCharacter4RecalcLevelSeedRecordBytesAnd-
 *   RebuildScenarioScena0800_801FDF84, which names 0x80144BF8 record 4's base and
 *   its bytes +0x0E..+0x11: record 5's base is 0x80144BF8 + 0xA4 = 0x80144C9C, so
 *   this body's base 0x80144CAA is exactly that record's +0x0E.
 *
 * Levers: the span is code, not data - it opens with its own `addiu $sp,$sp,
 *   -0x18` prologue and ends with its own `jr $ra` - so the data and carve
 *   levers do not apply and no layout probe is owed. Clone declined: one scan of
 *   the 396 extracted payloads under out/binaries for these 0xF8 bytes returned
 *   only this span, at payload offset 0x958 of emi/world04/area171/13.bin, byte
 *   for byte; no other shipped payload carries the body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte74Bit0ThenWhenParty5Status4BSetFlagBank73QueueUiBlock2x21To22SizeC0And3x21To22Size0BumpCountdown7EAndWhenParty7StartFrontend30ProgressAWorld04Area17113_801F3558(
    void) {
  u8* party;

  D_80146884[0x74] &= ~1u;
  party = &D_80145F09;
  if (*party == 5) {
    if (D_80144CAA[0] != 0x4B) {
      return;
    }
    func_8015B580(D_80144CAA + 0x27E, 0x73);
    func_801A4BC0(2, 0x21, 0xC0);
    func_801A4BC0(2, 0x22, 0xC0);
    func_801A4BC0(3, 0x21, 0);
    func_801A4BC0(3, 0x22, 0);
    *(u16*)(D_80146884 + 0x7E) += 1;
  }
  if (*party == 7) {
    func_8015C088();
    D_801448EB = 0x30;
    D_801448EC = 0xA;
  }
}
