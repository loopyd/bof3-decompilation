#include "bof3/bof3.h"

/* The leading data run of this payload starts at 0x800C1804 and holds 12-byte
 * slots whose first 0xA bytes are copied into a frame slot before use. This body
 * copies the second slot at 0x800C1810; the twin step body 0x800C27E8 copies the
 * first slot at 0x800C1804, and the alternative step body 0x800C305C copies the
 * one at 0x800C1858 inline. As in those lifts, the copy is one aggregate move:
 * both 4-byte groups travel through lwl/lwr and swl/swr pairs and the two
 * trailing bytes through lb/sb, which is what this compiler emits for a 10-byte
 * aggregate whose alignment is one, so the record type is a byte aggregate
 * rather than a plain u32 pair. The copied bytes are then indexed by the battle
 * halfword 0x801463BE, so the frame copy is read as bytes.
 */
typedef struct Boss05516StepTemplate {
  u8 bytes[0xA];
} Boss05516StepTemplate;

extern Boss05516StepTemplate D_800C1810; /* @source 0x800C1810 @kind table */

/* Shared battle work record pointer in main RAM, published outside this
 * overlay's image; this target's generated undefined_syms_auto.txt already
 * binds the address. The twin step body 0x800C27E8 reads the same record and
 * passes it through the byte view only, so this declaration keeps the byte view
 * too. This overlay's exact lift 0x800C2C78 spells the record's byte +1
 * GlobalWorkByte1, and the exact battle lifts of the other overlays spell byte
 * +0xB of the same record GlobalWorkByteB, so this lift uses that vocabulary for
 * the guard byte.
 */
extern u8* D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Battle halfword this overlay's step helpers use as the index into a step
 * template: the exact sibling lifts raise it, or store 8 into it to end a step
 * sequence, and it is not established which table of the battle state it
 * indexes. */
extern u16 D_801463BE; /* @source 0x801463BE @kind unknown */

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

/* @source 0x800C2948
 * @behavior Overlay step body, entry 1 of the two-entry handler pair that
 * dispatchStateByte2LocalPairBossBoss05516_800C2888 seeds with 0x800C28E0 then
 * this function, and the body entry 0 (0x800C28E0) also calls directly after its
 * own setup. It reads the shared battle work record pointer at 0x801EB4E0
 * before it adjusts the stack, copies the 10-byte step template at 0x800C1810
 * into frame slot 0x10, and then tests that record's unsigned byte +0xB: when
 * the byte is set it requests the template byte selected by the battle halfword
 * 0x801463BE through the shared resource request helper 0x8014D6B8, and when the
 * byte is clear it requests nothing. Either way it then calls the shared
 * readiness helper 0x801E30F8, the shared battle update 0x8014D4E0 and this
 * overlay's own work refresh 0x800C3284, in that order and discarding each
 * result, so this body writes no state of its own and the next step is picked by
 * the state byte the pair dispatcher reads. The template copy is one aggregate
 * move, all three loads ahead of the three stores, with the last byte loaded and
 * stored after the first store of the second word; the copy loads the block
 * before the guard and the index halfword is read after it, and the frame slot
 * address is formed in the guard branch's delay slot. This is the same 40
 * instruction body as the twin step body 0x800C27E8 and differs from it only in
 * the template slot it copies, so both names describe the same step-body role
 * for different step states. The alternative step body 0x800C305C carries this
 * same copy and tail inline with the template at 0x800C1858 and without the
 * guard, and it advances the recorded step byte itself, which this shared body
 * does not. No word of the payload holds 0x800C2948: the pair dispatcher
 * materializes it with lui plus addiu into its frame and 0x800C28E0 reaches it
 * with jal, so the pair and that caller are the only reaches and the 0x28-byte
 * frame holds the template at 0x10 and $ra at 0x20 across the calls. The roles
 * of work-record byte +0xB and of the template's own fields are not established,
 * so they stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyTemplateRecordThenRequestByteAtIndexedByBattleHalfwordWhenGlobalWorkByteBSetAndRefreshWorkBossBoss05516_800C2948(
    void) {
  u8* globalWork;
  Boss05516StepTemplate stepTemplate;

  globalWork = D_801EB4E0;
  stepTemplate = D_800C1810;
  if (globalWork[0xB] != 0) {
    func_8014D6B8(stepTemplate.bytes[D_801463BE]);
  }
  func_801E30F8();
  func_8014D4E0();
  refreshWorkFromRecordThenAdvancePositionByModeScaleBossBoss05516_800C3284();
}
