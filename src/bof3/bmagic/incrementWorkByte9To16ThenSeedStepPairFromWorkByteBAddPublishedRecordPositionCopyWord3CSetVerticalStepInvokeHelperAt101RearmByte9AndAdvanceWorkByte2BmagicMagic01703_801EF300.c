#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named fixed address so
 * every access group re-materializes the cell with its own lui plus offset
 * load, which is how the original reaches it once per store group; the original
 * re-reads it after every call and after every store through the loaded pointer
 * because a store may alias the cell itself. The exact siblings of this run
 * setWorkByte9To8AndAdvanceWorkByte2WhenHelperReadyBmagicMagic01703_801EF694
 * and invokeHelperThenDecrementWorkByte9ThenInvokeBattleHelpersAndAdvance
 * WorkByte2OnZeroWhenGlobalWorkByteBIs1BmagicMagic01703_801EF5FC read the same
 * cell the same way. */
extern u8 *volatile D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Published record pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt.
 * The cell is a pointer variable in main RAM, so the original re-reads it once
 * per statement below; declaring it volatile is what keeps those three separate
 * reads, because a plain extern lets CSE keep the first pointer value across
 * the work-object stores. The exact sibling at 0x801EF460 of this overlay's
 * handler run adds the same word pair 0x34/0x38 of a record onto the work
 * object, and copyWorkWordPairAndHalfwordClearBytes9AThenAdvanceWorkByte2Bmagic
 * Magic10303_801EEF94 measures its word pair 0x34/0x38 from this parallel
 * record published at 0x8014638C. */
extern u8 *volatile D_8014638C; /* @source 0x8014638C @kind unknown */

/* Motion helper of the battle image, outside this overlay's image, unlifted and
 * not semantically named anywhere in the tree, so it keeps its
 * address-qualified name. It is reached with the work object as its only
 * argument and returns nothing; the exact sibling
 * seedWorkObjectFromBattlerRecordWhenBattlerReadyAndStateByteNotSixElseFinish
 * WorkByte2BmagicMagic01200_801EF4B8 declares the same prototype before adding
 * record word 0x34/0x38 onto the work words this helper derives. */
extern void func_801DD350(u8 *work);

/* Main-executable helper at 0x8015D71C; this overlay passes the selector that
 * re-arms the work-byte-9 countdown. The exact sibling
 * oscillateWorkByte9UsingWorkByteBDirectionThenInvokeHelperAt101SetByte9To10
 * AndAdvanceWorkByte2WhenGlobalWorkByteBIsOneBmagicMagic06903_801EF1D0
 * declares the same prototype. */
void func_8015D71C(s32 arg0);

/* @source 0x801EF300
 * @behavior Step handler of this overlay's work-byte-2 handler run: the word at
 *           0x801EEC18 holds this function's address, index 2 of the ten words
 *           the run dispatcher func_801EF058 copies from 0x801EEC10 into its
 *           frame and indexes with scratchpad work-object byte 2, so the body
 *           runs while work byte 2 is 2. That dispatcher is itself the single
 *           entry of the run at 0x801EF990 which the work-byte-1 dispatcher
 *           dispatchWorkByte1HandlerBmagicMagic01703_801EF014 selects, and no
 *           jal in this payload names this function (the only occurrence of
 *           0x801EF300 in the payload is the table word), so the table word is
 *           its only incoming path. Per call it increments the work object's
 *           unsigned byte 9 in place (lbu / addiu 1 / sb) and tests the stored
 *           byte truncated to eight bits against 0x10, so the rest of the body
 *           runs only on the call that brings the countdown to sixteen. On that
 *           value it seeds the work words 0xC and 0x10 from bit 0 of work byte
 *           0xB: in both arms word 0xC takes 0xFFFE0000, while word 0x10 takes
 *           0xFFFF0000 when the bit is set and 0x00010000 when it is clear. It
 *           then invokes the shared helper func_801DD350 on the work object,
 *           re-reads the scratchpad cell, and stores work word 0x34 as the
 *           published record's word 0x34 added onto work word 0xC, work word
 *           0x38 as that record's word 0x38 added onto work word 0x10, and work
 *           word 0x3C as that record's word 0x3C. It then re-reads work byte
 *           0xB, masks bit 0 into the same local that carries the vertical step,
 *           zeroes work word 0xC and stores 0x1000 or -0x1000 into work word
 *           0x10, positive when the bit is set, and invokes func_801DD350 on the
 *           work object a second time. Finally, only while the work object's
 *           byte 0xB is zero, it invokes the main-executable helper at
 *           0x8015D71C with the selector 0x101, then stores 0x10 into work byte
 *           9 and increments work byte 2 with a byte-width add, advancing the
 *           run to index 3 at 0x801EF460. Every work-object access group
 *           re-materializes the scratchpad cell because a store may change the
 *           cell, and the record pointer cell is likewise re-read after each
 *           store. The 0x18-byte frame only keeps $ra at 0x10 across the three
 *           calls; the function takes no arguments, returns nothing through the
 *           jr $ra at 0x801EF458 and reads or writes no other state. The roles
 *           of work byte 9 beyond this countdown, of work byte 0xB beyond this
 *           direction bit, and of the words 0xC/0x10 beyond these seeded steps
 *           are not established, so the name describes the effect and the
 *           offsets stay anchored. The single local that first carries the
 *           record word then the masked direction bit is the shape that keeps
 *           the arm constants in one register across the branch, as the
 *           original does.
 * @status partial
 * @match 95.45
 * @residual 84 of 88 instructions match; both spans are 352 bytes, not byte
 *           identical. First difference is at +0xC4: the original loads record
 *           word 0x3C into $v1, while this build loads it into $v0. This build
 *           stores that word before loading the direction into $v0, emits bnez
 *           with the positive arm constant in its delay slot and omits the
 *           original beqz/j pair and branch-delay record-word store. Extra load
 *           delay nops leave the total size unchanged. The unresolved mechanism
 *           spans value allocation and delay-slot reorganization.
 */
void incrementWorkByte9To16ThenSeedStepPairFromWorkByteBAddPublishedRecordPositionCopyWord3CSetVerticalStepInvokeHelperAt101RearmByte9AndAdvanceWorkByte2BmagicMagic01703_801EF300(void) {
  u8 *work;
  u8 *source;
  u8 *motion;
  u32 step;

  if ((D_1F800044[9] += 1) == 0x10) {
    work = D_1F800044;
    if (work[0xB] & 1) {
      *(u32 *)(work + 0xC) = 0xFFFE0000;
      *(u32 *)(work + 0x10) = 0xFFFF0000;
    } else {
      *(u32 *)(work + 0xC) = 0xFFFE0000;
      *(u32 *)(work + 0x10) = 0x10000;
    }
    func_801DD350(D_1F800044);

    source = D_8014638C;
    motion = D_1F800044;
    *(u32 *)(motion + 0x34) = *(u32 *)(source + 0x34) + *(u32 *)(motion + 0xC);
    *(u32 *)(motion + 0x38) = *(u32 *)(D_8014638C + 0x38) + *(u32 *)(motion + 0x10);

    step = *(u32 *)(D_8014638C + 0x3C);
    *(u32 *)(motion + 0x3C) = step;

    step = motion[0xB];
    step &= 1;
    if (step) {
      step = 0x1000;
    } else {
      step = -0x1000;
    }
    *(u32 *)(motion + 0xC) = 0;
    *(u32 *)(motion + 0x10) = step;

    func_801DD350(D_1F800044);
    if (D_1F800044[0xB] == 0) {
      func_8015D71C(0x101);
    }
    D_1F800044[9] = 0x10;
    D_1F800044[2] += 1;
  }
}
