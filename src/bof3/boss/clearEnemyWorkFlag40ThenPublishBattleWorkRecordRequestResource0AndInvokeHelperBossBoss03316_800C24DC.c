#include "bof3/bof3.h"

/* Battle selection byte in main RAM, outside this overlay's image. The battle
 * targets own the address; this lift only reads it as the D_801EB630 record
 * index, the same selector the exact battle lift
 * clearPendingBattlerPairsExceptModeKind reads through D_80146374 - 3. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row is added here. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* 0x118-byte enemy work record image of the shared battle record base. The boss
 * lifts of this module pass D_801EB630 to copyOwnerRecordFieldsByMode, and the
 * exact battle lift clearPendingBattlerPairsExceptModeKind indexes the same base
 * as this record array selected by D_80146374 - 3. Only byte 0 is used here, so
 * every other byte stays padding. The stride is carried by the record type
 * rather than by an explicit scale in the subscript because the original keeps
 * the subtraction as its own instruction (addiu of -3 before the scale); an
 * explicit (index - 3) * 0x118 subscript folded that -3 into the base address
 * as a constant addend and one instruction shorter. */
typedef struct Boss033EnemyWork {
  u8 unk_00;
  u8 pad_01[0x117];
} Boss033EnemyWork;

/* Shared battle record base in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row is added here. */
extern Boss033EnemyWork D_801EB630[]; /* @source 0x801EB630 @kind unknown */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The battle lifts read the record it points at (its byte 0 flags, byte 0x05
 * and byte 0x48) and the exact battle lift initScratchWorkState publishes it at
 * the scratchpad cell the same way this handler does. The original loads the
 * pointer word itself, so the declaration is the pointer variable. Splat binds
 * the address through this target's generated undefined_syms_auto.txt, so no
 * target-map row is added here. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell: the battle code publishes the scratch
 * work record here and every access reaches it through this cell. Declared as a
 * named cell so each access folds to its own lui, which is what the original
 * does before the load that saves the cell and before both stores of the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Resource request helper of the main executable, reached through the generated
 * undefined_funcs_auto row this target already carries because its remaining asm
 * spans call 0x8014D6B8 too. The sibling boss and bmagic lifts declare the same
 * prototype and pass a resource id. */
extern void func_8014D6B8(u32 flag);

/* Shared main-executable helper 0x801E5988, declared per caller like the bmagic
 * lifts that tail-call it; this caller passes no arguments and uses no result. */
void func_801E5988(void);

/* @source 0x800C24DC
 * @behavior Work-step handler at index 6 of this overlay's code-pointer run
 *           based at 0x800C27B4, which is the base word the step dispatcher
 *           func_800C1E70 loads and indexes with work byte 2 scaled by four; run
 *           word 6 at 0x800C27CC holds this address, and no jal in this payload
 *           names it. It first reads the battle selection byte 0x80146374,
 *           subtracts three from it, scales it by the 0x118-byte enemy work
 *           record stride and clears bit 0x40 of that record's byte 0 through
 *           the 0xBF mask, re-materialising the record base for the read and
 *           again for the store. It then saves the scratchpad work-object
 *           pointer cell 0x1F800044, publishes the shared battle work record
 *           pointer held at 0x801EB4E0 there, requests resource 0 through the
 *           main-executable wrapper func_8014D6B8, restores the previously
 *           published work object and runs the shared helper func_801E5988,
 *           which takes no arguments. It advances no work byte, so this step
 *           stays selected. Takes no arguments and returns nothing; the
 *           0x18-byte frame only keeps $ra and the saved work object across the
 *           two calls. The roles of the enemy work records byte 0 bits and of
 *           the published record are not established here, so the record bytes
 *           stay named by offset.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearEnemyWorkFlag40ThenPublishBattleWorkRecordRequestResource0AndInvokeHelperBossBoss03316_800C24DC(
    void) {
  u8 *record;
  u8 *saved;

  D_801EB630[D_80146374 - 3u].unk_00 &= 0xBF;
  record = D_801EB4E0;
  saved = D_1F800044;
  D_1F800044 = record;
  func_8014D6B8(0);
  D_1F800044 = saved;
  func_801E5988();
}
