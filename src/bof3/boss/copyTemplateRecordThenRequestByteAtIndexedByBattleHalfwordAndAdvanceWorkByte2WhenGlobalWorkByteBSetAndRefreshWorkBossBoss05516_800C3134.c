#include "bof3/bof3.h"

/* The leading data run of this payload starts at 0x800C1804 and holds 12-byte
 * slots whose first 0xA bytes are copied into a frame slot before use. The
 * alternative step body at 0x800C305C and this body both copy the slot at
 * 0x800C1858, while the twin bodies at 0x800C27E8, 0x800C2948 and 0x800C2AA8
 * copy the slot at 0x800C1804 and 0x800C1810. The bytes here are
 * 00 00 00 00 00 00 00 02 01 03. The copy is one aggregate move: both 4-byte
 * groups travel through lwl/lwr and swl/swr pairs and the two trailing bytes
 * through lb/sb, which is the code this compiler emits for a 10-byte aggregate
 * whose alignment is one, so the record type is a byte aggregate rather than a
 * plain u32 pair. The copied bytes are then indexed by the battle halfword
 * 0x801463BE, so the frame copy is read as bytes.
 */
typedef struct Boss05516StepTemplate {
  u8 bytes[0xA];
} Boss05516StepTemplate;

extern Boss05516StepTemplate D_800C1858; /* @source 0x800C1858 @kind table */

/* Shared battle work record pointer in main RAM, published outside this
 * overlay's image; this target's generated undefined_syms_auto.txt already
 * binds the address. The exact sibling lifts of this overlay read the same
 * record and pass it through the byte view only, so this declaration keeps the
 * byte view too. This overlay's exact lift 0x800C2C78 spells the record's byte
 * +1 GlobalWorkByte1, and the exact battle lifts of the other overlays spell
 * byte +0xB of the same record GlobalWorkByteB, so this lift uses that
 * vocabulary for the guard byte. */
extern u8* D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Battle halfword this overlay's step helpers use as the index into a step
 * template: the exact sibling lifts raise it, or store 8 into it to end a step
 * sequence, and it is not established which table of the battle state it
 * indexes. */
extern u16 D_801463BE; /* @source 0x801463BE @kind unknown */

/* Scratchpad cell holding the current work record, whose unsigned byte +2 is
 * the state byte the payload handler run indexes. The exact sibling lifts of
 * this overlay declare the same non-volatile pointer and advance the byte by
 * one to move to the next state body. */
extern u8* g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared main-executable resource request helper; the sibling battle, world and
 * plchar lifts declare the same prototype and pass a resource id. */
extern void func_8014D6B8(u32 flag); /* @source 0x8014D6B8 */

/* Shared main-executable readiness helper, called here without arguments and
 * with its result discarded. */
extern u8 func_801E30F8(void); /* @source 0x801E30F8 */

/* Shared main-executable battle update, called without arguments. Its role
 * beyond that is not established, so it keeps its address-anchored spelling. */
extern void func_8014D4E0(void); /* @source 0x8014D4E0 */

/* This overlay's own work-refresh step, defined in its own source file. */
extern void refreshWorkFromRecordThenAdvancePositionByModeScaleBossBoss05516_800C3284(void); /* @source 0x800C3284 */

/* @source 0x800C3134
 * @behavior Overlay step body and middle entry of the three alternative step
 * bodies that the payload table at 0x800C184C holds (0x800C305C, this function
 * and 0x800C31F0) and that dispatchStateByte2TableCopyBossBoss05516_800C2FFC
 * selects with the work record's state byte +2. It reads the shared battle work
 * record pointer at 0x801EB4E0 before it adjusts the stack, copies the 10-byte
 * step template at 0x800C1858 into frame slot 0x10, and then tests that
 * record's unsigned byte +0xB: when the byte is set it requests the template
 * byte selected by the battle halfword 0x801463BE through the shared resource
 * request helper 0x8014D6B8 and then advances the state byte +2 of the work
 * record at scratchpad cell 0x1F800044 by one, and when the byte is clear it
 * requests nothing and advances nothing. Either way it then calls the shared
 * readiness helper 0x801E30F8, the shared battle update 0x8014D4E0 and this
 * overlay's own work refresh 0x800C3284, in that order and discarding each
 * result. The advance sits inside the guard, after the request call, and the
 * pointer cell is re-read there because that call may alias it; the template
 * copy is one aggregate move, all three loads ahead of the three stores, with
 * the last byte loaded and stored after the first store of the second word, the
 * copy loads the block before the guard, the frame slot address is formed in
 * the guard branch's delay slot and the index halfword is read only inside the
 * guard. The alternative step body 0x800C305C carries the same template copy
 * inline with no guard: it also requests 0x8014D6B8 but resources 0x318 and
 * clears work-record bytes +0x48 and +0x24 and word +0x3C first, and it
 * advances the state byte only after its tail. The three-entry table makes
 * these bodies alternatives rather than a sequence, so the state byte is what
 * re-enters the dispatch. The 0x28-byte frame holds the template at 0x10 and
 * $ra at 0x20 across the calls. The roles of work-record byte +0xB, of the
 * template's own fields and of the battle halfword used as the template index
 * are not established, so they stay offset- and address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyTemplateRecordThenRequestByteAtIndexedByBattleHalfwordAndAdvanceWorkByte2WhenGlobalWorkByteBSetAndRefreshWorkBossBoss05516_800C3134(
    void) {
  u8* globalWork;
  Boss05516StepTemplate stepTemplate;

  globalWork = D_801EB4E0;
  stepTemplate = D_800C1858;
  if (globalWork[0xB] != 0) {
    func_8014D6B8(stepTemplate.bytes[D_801463BE]);
    g_battle_work[2]++;
  }
  func_801E30F8();
  func_8014D4E0();
  refreshWorkFromRecordThenAdvancePositionByModeScaleBossBoss05516_800C3284();
}
