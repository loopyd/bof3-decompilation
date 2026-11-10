#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 D_801448EB; /* @source 0x801448EB @kind unknown */
extern u8 D_801448EC; /* @source 0x801448EC @kind unknown */
extern u8 D_801448ED; /* @source 0x801448ED @kind unknown */
extern u8 *D_80148208; /* @source 0x80148208 @kind unknown */
extern u8 D_801F4F94[]; /* @source 0x801F4F94 @kind table */
extern u8 D_801F4F95[]; /* @source 0x801F4F95 @kind table */

void func_8015C088(void);

/* @source 0x801F4764
 * @behavior Stores 0xFFFF into the shared status halfword at 0x801490A8 on
 *           every path (`ori v0,0,0xFFFF`, `sh`), and only while the shared
 *           gate byte at 0x801490C7 equals 3 (`lb`, so the cell is s8) runs the
 *           shared front-end startup helper func_8015C088 and then writes the
 *           shared state-entry tuple: 0x801448EB = 0x0A, 0x801448EC = 5 and
 *           0x801448ED = 0xFF, each from its own `addiu`. It then copies the two
 *           bytes of the row the gate byte selects from the payload-resident
 *           stride-2 row table at 0x801F4F94 into the words at +0x18 and +0x1C
 *           of the record pointer held at D_80148208; the second copy takes the
 *           0x801F4F95 byte of the same row. The gate byte is read again for
 *           that second row, which the fixed-address accessor at that use site
 *           reproduces. The gate load is hoisted above the status store, `bne`
 *           jumps past the tuple stores, so gate 3 is the arm that falls
 *           through, and the 0x18-byte frame only keeps $ra across the helper
 *           call. Takes no arguments and returns nothing.
 *
 * Name evidence: 0x801448EB is the state-entry byte of the shared tuple the
 * setStatusFFFFAndStateEntry* handlers write, with 0x801448EC as the progress
 * byte and 0x801448ED as its companion byte, 0x801490C7 is the shared gate byte
 * those handlers branch on, and 0x8015C088 is the shared front-end startup
 * helper the whole world family runs. 0x801F4F94 and 0x801F4F95 are the two
 * byte columns of a four-row, stride-2 table in this payload's own textbin tail
 * (row bytes 4/5/7/9 then 0), the same gate-row role the sibling source
 * setStatusFFFFAndPublishRecordWords18And1CFromGateRowWorld04Area19213_801F2CFC
 * gives the 0x801F380C pair in its overlay. The payload names this body only
 * from a table word: 0x801F4F48 holds 0x801F4764 inside the handler run
 * 0x801F4F30..0x801F4F4C, and no in-image `jal` reaches it, so its roles stay
 * structural. 0x801448ED is a shared scratch cell other paths reuse as a commu
 * record row, so the companion-byte vocabulary is convention rather than a
 * proven role.
 *
 * Shape: statement order, operand forms, offset spelling and declaration style
 * are taken from the near-identical sibling
 * setStatusFFFFAndPublishRecordWords18And1CFromGateRowWorld04Area19213_801F2CFC,
 * whose 0x58-byte body carried the identical row-publish tail with the same
 * `lui $at`/`addu $at,$at,index` fold and had no gate-3 tuple block, together
 * with the in-target sibling
 * setStatusFFFFAndStateEntryWhenGateClearWorld04Area16813_801F4808, whose
 * 0x40-byte body carried the same status store and gated byte-store group. Both
 * are shape evidence rather than clone sources. No byte-identical twin exists:
 * the span matches no other payload under out/binaries, and its row-table
 * immediates are in-payload data words no other overlay can carry.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndStateEntryAProgress5SubstateFFWhenGate3ThenPublishRecordFromGateRowWorld04Area16813_801F4764(
    void) {
  u8 *record;

  D_801490A8 = 0xFFFF;
  if (D_801490C7 == 3) {
    func_8015C088();
    D_801448EB = 0xA;
    D_801448EC = 0x5;
    D_801448ED = 0xFF;
  }
  record = D_80148208;
  *(u32 *)(record + 0x18) = D_801F4F94[D_801490C7 * 2];
  *(u32 *)(record + 0x1C) = D_801F4F95[PSX_REF(s8, 0x801490C7u) * 2];
}
